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

def FullOTA_Assertions(info):
  #TODO: Implement device specific asserstions.
  return

def IncrementalOTA_Assertions(info):
  #TODO: Implement device specific asserstions.
  return

# This function handles only non-HLOS boot images - files list must contain
# only such images (aboot, tz, etc)
def ExtractBootFiles(info, input_zip, files, filesmap):
  for f in files:
    fn = f
    # is the file encoded?
    enc = f.endswith('.enc')
    if enc:
      # disregard the .enc extention
      fn = f[:-4]

    if fn not in filesmap:
      print "warning radio-update: '%s' not found in filesmap" % (fn)
      continue

    filepath = "firmware-update/" + f
    image_data = input_zip.read(files[f])
    common.ZipWriteStr(info.output_zip, filepath, image_data)

    if enc:
      info.script.AppendExtra('package_extract_file("%s", "/tmp/%s");' % (filepath, f))
      info.script.AppendExtra('msm.decrypt("/tmp/%s", "%s");' % (f, filesmap[fn]))
    else:
      info.script.AppendExtra('package_extract_file("%s", "%s");' % (filepath, filesmap[fn]))
  return

# This function handles only non-HLOS boot images - files list must contain
# only such images (aboot, tz, etc)
def InstallBootImages(info, input_zip, files, filesmap):
  filesmapBak = {}
  for f in filesmap:
    if f.endswith('.bak'):
      filesmapBak[f[:-4]] = filesmap[f]

  #update main partitions
  info.script.AppendExtra('ifelse(msm.boot_update("main"), (')
  info.script.AppendExtra('ui_print("Main boot images update start");')

  ExtractBootFiles(info, input_zip, files, filesmap)

  info.script.AppendExtra('ui_print("Main boot images update end")')
  info.script.AppendExtra('), ui_print("Main boot images already updated") );')

  #update backup partitions
  info.script.AppendExtra('ifelse(msm.boot_update("backup"), (')
  info.script.AppendExtra('ui_print("Backup boot images update start");')

  if filesmapBak != {}:
    ExtractBootFiles(info, input_zip, files, filesmapBak)

  info.script.AppendExtra('ui_print("Backup boot images update end")')
  info.script.AppendExtra('), ui_print("Backup boot images already updated") );')

  #finalize partitions update
  info.script.AppendExtra('msm.boot_update("finalize");')
  return

# This function handles only non-HLOS firmware images different from boot
# images - files list must contain only such images (modem, dsp, etc)
def InstallFwImages(info, input_zip, files, filesmap):
  last_mounted = ""

  for f in files:
    filetoken = f
    # If full file name is not specified in filesmap get only the name part
    # and look for this token
    if f not in filesmap:
      filetoken = f.split(".")[0] + ".*"
      if filetoken not in filesmap:
        print "warning radio-update: '%s' not found in filesmap" % (filetoken)
        continue

    filepath = "firmware-update/" + f
    image_data = input_zip.read(files[f])
    common.ZipWriteStr(info.output_zip, filepath, image_data)

    fn = f
    # is the file encoded?
    enc = f.endswith('.enc')
    if enc:
      # disregard the .enc extention
      fn = f[:-4]

    dest = ""
    # Parse filesmap destination paths for "/dev/" pattern at the beginng.
    # This would mean that the file must be written to block device -
    # mount needed
    mount = filesmap[filetoken].startswith("/dev/")
    if mount:
      if last_mounted != filesmap[filetoken]:
        info.script.AppendExtra('unmount("/firmware");')
        info.script.AppendExtra('mount("vfat", "EMMC", "%s", "/firmware");' % (filesmap[filetoken]))
        last_mounted = filesmap[filetoken]
      dest = "/firmware/image/" + fn
    else:
      dest = filesmap[filetoken] + "/" + fn

    if enc:
      info.script.AppendExtra('package_extract_file("%s", "/tmp/%s");' % (filepath, f))
      info.script.AppendExtra('msm.decrypt("/tmp/%s", "%s");' % (f, dest))
    else:
      info.script.AppendExtra('package_extract_file("%s", "%s");' % (filepath, dest))

  if last_mounted != "":
    info.script.AppendExtra('unmount("/firmware");')
  return

def InstallRawImage(image_data, fn, info, filesmap):
  if fn not in filesmap:
    return
  info.script.AppendExtra('package_extract_file("%s", "%s");' % (fn, filesmap[fn]))
  common.ZipWriteStr(info.output_zip, fn, image_data)
  return

def InstallBinImages(info, input_zip, files, filesmap):
  for f in files:
    image_data = input_zip.read(files[f])
    InstallRawImage(image_data, f, info, filesmap)
  return

def FullOTA_InstallEnd_MMC(info):
  files = GetRadioFiles(info.input_zip)
  if files == {}:
    print "warning radio-update: no radio images in input target_files"
    return

  filesmap = LoadFilesMap(info.input_zip)
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

  if binFiles != {}:
    InstallBinImages(info, info.input_zip, binFiles, filesmap)
  if bootFiles != {}:
    InstallBootImages(info, info.input_zip, bootFiles, filesmap)
  if fwFiles != {}:
    InstallFwImages(info, info.input_zip, fwFiles, filesmap)
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

  info.script.UnmountAll()
  return

def WriteRadioFirmware(info, f, files, filesmap, trgt_img, src_img=None):
  tf = common.File(files[f], trgt_img)

  if src_img is None:
    tf.AddToZip(info.output_zip)
    InstallRawImage(trgt_img, f, info, filesmap)
  else:
    sf = common.File(files[f], src_img)
    if tf.sha1 == sf.sha1:
      print "%s image unchanged; skipping" % (f)
    else:
      diff = common.Difference(tf, sf)
      common.ComputeDifferences([diff])
      _, _, d = diff.GetPatch()
      if d is None or len(d) > tf.size * common.OPTIONS.patch_threshold:
        # compute difference failed or the difference is bigger than
        # target file - write the whole target file.
        tf.AddToZip(info.output_zip)
        InstallRawImage(trgt_img, f, info, filesmap)
      else:
        patch = "patch/" + f + ".p"
        common.ZipWriteStr(info.output_zip, patch, d)
        info.script.Print("Patching Radio...")
        radio_device = filesmap[f]
        info.script.ApplyPatch("EMMC:%s:%d:%s:%d:%s" %
                (radio_device, sf.size, sf.sha1, tf.size, tf.sha1),
                "-", tf.size, tf.sha1, sf.sha1, patch)
  return

def IncrementalOTA_InstallEnd_MMC(info):
  files = GetRadioFiles(info.target_zip)
  if files == {}:
    print "warning radio-update: no radio images in input target_files"
    return

  filesmap = LoadFilesMap(info.target_zip)
  if filesmap == {}:
    print "warning radio-update: no or invalid filesmap file found"
    return

  info.script.Print("Incrementally updating firmware images...")

  fwFiles = {}
  bootFiles = {}
  binFiles = {}
  incFiles = {}

  # Separate image types as each type needs different handling
  for f in files:
    dotSeparatedFname = f.split(".")
    if dotSeparatedFname[-1] == 'enc':
      if dotSeparatedFname[-2] == 'mbn':
        bootFiles[f] = files[f]
      elif dotSeparatedFname[-2] == 'bin':
        binFiles[f] = files[f]
      else:
        fwFiles[f] = files[f]
    else:
      incFiles[f] = files[f]

  if binFiles != {}:
    InstallBinImages(info, info.target_zip, binFiles, filesmap)
  if bootFiles != {}:
    InstallBootImages(info, info.target_zip, bootFiles, filesmap)
  if fwFiles != {}:
    InstallFwImages(info, info.target_zip, fwFiles, filesmap)

  for f in incFiles:
    try:
      trgt_img = info.target_zip.read(incFiles[f])
      try:
        src_img = info.source_zip.read(incFiles[f])
      except KeyError:
        print "source radio image = None"
        src_img = None

      WriteRadioFirmware(info, f, files, filesmap, trgt_img, src_img)
    except KeyError:
      print "no %s in target target_files; skipping install" % (f)
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

  info.script.UnmountAll()
  return
