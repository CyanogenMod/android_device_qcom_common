# Copyright (C) 2009 The Android Open Source Project
# Copyright (c) 2011, The Linux Foundation. All rights reserved.
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

def LoadFilesMap(zip, type=None):
  try:
    data = zip.read("RADIO/filesmap")
  except KeyError:
    print "Warning: could not find RADIO/filesmap in %s." % zip
    data = ""
  d = {}
  for line in data.split("\n"):
    line = line.strip()
    if not line or line.startswith("#"): continue
    pieces = line.split()
    if not (len(pieces) == 2):
      raise ValueError("malformed filesmap line: \"%s\"" % (line,))
    d[pieces[0]] = pieces[1]
  return d

def GetRadioFiles(z):
  out = {}
  for info in z.infolist():
    if info.filename.startswith("RADIO/") and (info.filename.__len__() > len("RADIO/")):
      fn = "RADIO/" + info.filename[6:]
      out[fn] = fn
  return out

def FullOTA_Assertions(info):
  #TODO: Implement device specific asserstions.
  return

def IncrementalOTA_Assertions(info):
  #TODO: Implement device specific asserstions.
  return

def InstallRawImage(image_data, api_version, input_zip, fn, info, filesmap):
  #fn is in RADIO/* format. Extracting just file name.
  filename = fn[6:]
  if api_version >= 3:
    if filename not in filesmap:
        return
    info.script.AppendExtra('package_extract_file("%s", "%s");' % (filename,filesmap[filename]))
    common.ZipWriteStr(info.output_zip, filename, image_data)
    return
  else:
    print "warning raido-update: no support for api_version less than 3."

def FULLOTA_InstallEnd_MMC(info):
  files = GetRadioFiles(info.input_zip)
  if files == {}:
    print "warning radio-update: no radio image in input target_files; not flashing radio"
    return
  info.script.UnmountAll()
  info.script.Print("Writing radio image...")
  #Load filesmap file
  filesmap = LoadFilesMap(info.input_zip, info.type)
  if filesmap == {}:
      print "warning radio-update: no or invalid filesmap file found. not flashing radio"
      return
  for f in files:
    image_data = info.input_zip.read(f)
    InstallRawImage(image_data, info.input_version, info.input_zip, f, info, filesmap)
  return

def FULLOTA_InstallEnd_MTD(info):
  print "warning radio-update: no implementation for radio upgrade for NAND devices"
  return

def FullOTA_InstallEnd(info):
  if info.type == 'MTD':
    FULLOTA_InstallEnd_MTD(info)
  if info.type == 'MMC':
    FULLOTA_InstallEnd_MMC(info)

def WriteRadioFirmware(info, filename, target_radio_img, source_radio_img=None):
  fn = filename[6:]
  filesmap = LoadFilesMap(info.target_zip)
  if filesmap == {}:
    print "warning radio-update: no or invalid filesmap file found. Not flashing %s" % (fn)
    return

  tf = common.File(filename, target_radio_img)

  if source_radio_img is None:
    tf.AddToZip(info.output_zip)
    info.script.Print("Writing radio...")
    InstallRawImage(target_radio_img, info.target_version, info.target_zip, filename, info, filesmap)
  else:
    sf = common.File(filename, source_radio_img);
    if tf.sha1 == sf.sha1:
      print "%s image unchanged; skipping" % (fn)
    else:
      diff = common.Difference(tf, sf)
      common.ComputeDifferences([diff])
      _, _, d = diff.GetPatch()
      if d is None or len(d) > tf.size * common.OPTIONS.patch_threshold:
        # compute difference failed or the difference is bigger than
        # target file - write the whole target file.
        tf.AddToZip(info.output_zip)
        info.script.Print("Writing radio ...")
        InstallRawImage(target_radio_img, info.target_version, info.target_zip, filename, info, filesmap)
      else:
        PatchName = fn + ".p"
        common.ZipWriteStr(info.output_zip, "patch/" + PatchName, d)
        info.script.Print("Patching Radio...")
        if info.type == 'MMC':
          print "-------- EMMC device given -------"
          radio_device = filesmap[fn]
          info.script.ApplyPatch("%s:%s:%d:%s:%d:%s" %
                               ("EMMC", radio_device, sf.size, sf.sha1, tf.size, tf.sha1),
                               "-", tf.size, tf.sha1, sf.sha1, "patch/" + PatchName)
        else:
          print "-------- MTD device given - not supported --------"

  return

def IncrementalOTA_InstallEnd(info):
  files = GetRadioFiles(info.target_zip)
  if files == {}:
    print "warning radio-update: no radio images in input target_files; not flashing radio"
    return

  filesmap = LoadFilesMap(info.target_zip)
  if filesmap == {}:
    print "warning radio-update: no or invalid filesmap file found. not flashing radio"
    return

  info.script.Print("Applying incremental radio image...")

  for f in files:
    try:
      if f == "RADIO/filesmap": continue
      target_radio_img = info.target_zip.read(f)
      try:
        source_radio_img = info.source_zip.read(f)
      except KeyError:
        print "source radio image = None"
        source_radio_img = None

      WriteRadioFirmware(info, f, target_radio_img, source_radio_img)
    except KeyError:
      print "no %s in target target_files; skipping install" % (f)

  return
