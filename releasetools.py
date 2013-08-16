# Copyright (C) 2009 The Android Open Source Project
# Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Emit commands needed for QCOM devices during OTA installation
(installing the radio image)."""

import common
import re

def LoadFilesMap(zip, name="RADIO/filesmap"):
  try:
    data = zip.read(name)
  except KeyError:
    print "Warning: could not find %s in %s." % (name, zip)
    data = ""
  d = {}
  for line in data.split("\n"):
    line = line.strip()
    if not line or line.startswith("#"):
      continue
    pieces = line.split()
    if not (len(pieces) == 2):
      raise ValueError("malformed filesmap line: \"%s\"" % (line,))
    d[pieces[0]] = pieces[1]
  return d

def GetRadioFiles(z):
  out = {}
  for info in z.infolist():
    if info.filename.startswith("RADIO/") and (info.filename.__len__() > len("RADIO/")):
      fn = info.filename[6:]
      if fn.startswith("filesmap"):
        continue
      out[fn] = info.filename
  return out

def GetImageData(fpath, in_zip, base_zip=None):
  try:
    in_img = in_zip.read(fpath)
  except KeyError:
    print "Warning: could not read '%s' from '%s'." % (fpath, in_zip)
    in_img = ""

  base_img = ""
  if base_zip is not None:
    try:
      base_img = base_zip.read(fpath)
    except KeyError:
      base_img = ""
  return in_img, base_img

def FullOTA_Assertions(info):
  #TODO: Implement device specific asserstions.
  return

def IncrementalOTA_Assertions(info):
  #TODO: Implement device specific asserstions.
  return

# This function checks if fw image is to be handled incrementally and if
# this is the case it creates and adds the diff patch to the update zip and
# adds the appropriate applypatch command to the update script
def HandleIncrementally(info, f, fpath, img_dest, in_img, base_img):
  in_f = common.File(fpath, in_img)
  base_f = common.File(fpath, base_img)
  if in_f.sha1 == base_f.sha1:
    return False

  diff = common.Difference(in_f, base_f)
  common.ComputeDifferences([diff])
  _, _, d = diff.GetPatch()
  if d is None or len(d) > in_f.size * common.OPTIONS.patch_threshold:
    # compute difference failed or the difference is bigger than the
    # target file * predefined threshold - do full update
    return False

  patch = "patch/firmware-update/" + f + ".p"
  common.ZipWriteStr(info.output_zip, patch, d)
  info.script.ApplyPatch("EMMC:%s:%d:%s:%d:%s" %
          (img_dest, base_f.size, base_f.sha1, in_f.size, in_f.sha1),
          "-", in_f.size, in_f.sha1, base_f.sha1, patch)
  return True

# This function handles only non-HLOS whole partition images - files list
# must contain only such images
def InstallRawImages(info, files, filesmap, in_zip, base_zip=None):
  for f in files:
    in_img, base_img = GetImageData(files[f], in_zip, base_zip)
    if in_img == "":
      continue

    fn = f
    # is the file encoded?
    enc = f.endswith('.enc')
    if enc:
      # disregard the .enc extention
      fn = f[:-4]

    if fn not in filesmap:
      print "warning radio-update: '%s' not found in filesmap" % (fn)
      continue

    # handle incrementally?
    if base_img != "" and not enc:
      if HandleIncrementally(info, fn, files[f], filesmap[fn], in_img, base_img):
        continue

    filepath = "firmware-update/" + f
    common.ZipWriteStr(info.output_zip, filepath, in_img)

    if enc:
      info.script.AppendExtra('package_extract_file("%s", "/tmp/%s");' % (filepath, f))
      info.script.AppendExtra('msm.decrypt("/tmp/%s", "%s");' % (f, filesmap[fn]))
    else:
      info.script.AppendExtra('package_extract_file("%s", "%s");' % (filepath, filesmap[fn]))
  return

# This function handles only non-HLOS boot images - files list must contain
# only such images (aboot, tz, etc)
def InstallBootImages(info, files, filesmap, in_zip, base_zip=None):
  filesmapBak = {}
  for f in filesmap:
    if f.endswith('.bak'):
      filesmapBak[f[:-4]] = filesmap[f]

  # update main partitions
  info.script.AppendExtra('ifelse(msm.boot_update("main"), (')
  InstallRawImages(info, files, filesmap, in_zip, base_zip)
  info.script.AppendExtra('ui_print("Main boot images updated") ), "");')

  if filesmapBak != {}:
    # update backup partitions
    info.script.AppendExtra('ifelse(msm.boot_update("backup"), (')
    InstallRawImages(info, files, filesmapBak, in_zip, base_zip)
    info.script.AppendExtra('ui_print("Backup boot images updated") ), "");')
  else:
    info.script.AppendExtra('msm.boot_update("backup");')

  # finalize partitions update
  info.script.AppendExtra('msm.boot_update("finalize");')
  return

# This function handles only non-HLOS firmware images different from boot
# images - files list must contain only such images (modem, dsp, etc)
def InstallFwImages(info, files, filesmap, in_zip, base_zip=None):
  last_mounted = ""

  for f in files:
    in_img, base_img = GetImageData(files[f], in_zip, base_zip)
    if in_img == "":
      continue

    fn = f
    # is the file encoded?
    enc = f.endswith('.enc')
    if enc:
      # disregard the .enc extention
      fn = f[:-4]

    filetoken = fn
    # If full file name is not specified in filesmap get only the name part
    # and look for this token
    if fn not in filesmap:
      filetoken = fn.split(".")[0] + ".*"
      if filetoken not in filesmap:
        print "warning radio-update: '%s' not found in filesmap" % (filetoken)
        continue

    dest = ""
    # Parse filesmap destination paths for "/dev/" pattern at the beginng.
    # This would mean that the file must be written to block device -
    # fs mount needed
    mount = filesmap[filetoken].startswith("/dev/")
    if mount:
      if last_mounted != filesmap[filetoken]:
        info.script.AppendExtra('unmount("/firmware");')
        info.script.AppendExtra('mount("vfat", "EMMC", "%s", "/firmware");' % (filesmap[filetoken]))
        last_mounted = filesmap[filetoken]
      dest = "/firmware/image/" + fn
    else:
      dest = filesmap[filetoken] + "/" + fn

    # handle incrementally?
    if base_img != "" and not enc:
      if HandleIncrementally(info, fn, files[f], dest, in_img, base_img):
        continue

    filepath = "firmware-update/" + f
    common.ZipWriteStr(info.output_zip, filepath, in_img)

    if enc:
      info.script.AppendExtra('package_extract_file("%s", "/tmp/%s");' % (filepath, f))
      info.script.AppendExtra('msm.decrypt("/tmp/%s", "%s");' % (f, dest))
    else:
      info.script.AppendExtra('package_extract_file("%s", "%s");' % (filepath, dest))

  if last_mounted != "":
    info.script.AppendExtra('unmount("/firmware");')
  return

def OTA_InstallEnd_MMC(info, in_zip, base_zip=None):
  files = GetRadioFiles(in_zip)
  if files == {}:
    print "warning radio-update: no radio images in input target_files"
    return

  filesmap = LoadFilesMap(in_zip)
  if filesmap == {}:
    print "warning radio-update: no or invalid filesmap file found"
    return

  info.script.Print("Updating firmware images...")

  fwFiles = {}
  bootFiles = {}
  binFiles = {}

  # Separate image types as each type needs different handling
  for f in files:
    dotSeparatedFname = f.split(".")
    if dotSeparatedFname[-1] == 'enc':
      if dotSeparatedFname[-2] == 'mbn':
        bootFiles[f] = files[f]
      else:
        fwFiles[f] = files[f]
    elif dotSeparatedFname[-1] == 'mbn':
      bootFiles[f] = files[f]
    elif dotSeparatedFname[-1] == 'bin':
      binFiles[f] = files[f]
    else:
      fwFiles[f] = files[f]

  if bootFiles != {}:
    InstallBootImages(info, bootFiles, filesmap, in_zip, base_zip)
  if binFiles != {}:
    InstallRawImages(info, binFiles, filesmap, in_zip, base_zip)
  if fwFiles != {}:
    InstallFwImages(info, fwFiles, filesmap, in_zip, base_zip)
  return

def FullOTA_InstallEnd_MMC(info):
  OTA_InstallEnd_MMC(info, info.input_zip)
  return

def FullOTA_InstallEnd_MTD(info):
  print "warning radio-update: radio update for NAND devices not supported"
  return

def FullOTA_InstallEnd(info):
  if info.input_version < 3:
    print "warning radio-update: no support for api_version less than 3"
  else:
    if info.type == 'MMC':
      FullOTA_InstallEnd_MMC(info)
    elif info.type == 'MTD':
      FullOTA_InstallEnd_MTD(info)
  return

def IncrementalOTA_InstallEnd_MMC(info):
  OTA_InstallEnd_MMC(info, info.target_zip, info.source_zip)
  return

def IncrementalOTA_InstallEnd_MTD(info):
  print "warning radio-update: radio update for NAND devices not supported"
  return

def IncrementalOTA_InstallEnd(info):
  if info.target_version < 3:
    print "warning radio-update: no support for api_version less than 3"
  else:
    if info.type == 'MMC':
      IncrementalOTA_InstallEnd_MMC(info)
    elif info.type == 'MTD':
      IncrementalOTA_InstallEnd_MTD(info)
  return
