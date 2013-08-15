Copyright (c) 2013, The Linux Foundation. All rights reserved.

Redistribution and use in source form and compiled forms (SGML, HTML,
PDF, PostScript, RTF and so forth) with or without modification, are
permitted provided that the following conditions are met:

Redistributions in source form must retain the above copyright
notice, this list of conditions and the following disclaimer as the
first lines of this file unmodified.

Redistributions in compiled form (transformed to other DTDs,
converted to PDF, PostScript, RTF and other formats) must reproduce
the above copyright notice, this list of conditions and the following
disclaimer in the documentation and/or other materials provided with
the distribution.

THIS DOCUMENTATION IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE AND
NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT SHALL THE FREEBSD
DOCUMENTATION PROJECT BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS DOCUMENTATION, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.


-------------------------------------------------------------
-------------------------------------------------------------


Introduction
----------
README.txt describes the parser usage for display component in
GCDB(Global Component Database).

Usage:
---------
Display parser supports generating dtsi and header file for android
kernel and LK.

command: perl parser.pl <.xml> <panel/platform>

Next two sections descrbe the usage of parser script for xml files
preset in this folder. User can follow same for their own panel
or target files.


Generate Panel dtsi/header file
------------------------
User should use below command to generate the panel dtsi/header file for
panel_nt35590_720p_cmd.xml. This xml file is part of this folder.

#perl parser.pl panel_nt35590_720p_cmd.xml panel

It generates dsi-panel-nt35590-720p-cmd.dtsi and panle_nt35590_720p_cmd.h
files. Dtsi file should be copied to dts folder in kernel while header
file should be copied to bootloader GCDB header file database.

It also contains the video mode panel xml file.

#perl parser.pl panel_nt35596_1080p_video.xml panel

It generates dsi-panel-nt35596-1080p-video.dtsi and panle_nt35596_1080p_video.h
files. Dtsi file should be copied to dts folder in kernel while header file
should be copied to bootloader GCDB header file database.



Generate Platform dtsi/header file
---------------------------
User should use below command to generate the display platform dtsi/header file
for platform-msm8610.xml. This XML file is part of this folder.

#perl parser.pl platform-msm8610.xml platform

It generates platform_msm8610.h and platform-msm8610.dtsi files. The content of
dtsi file should be copied to <target-mdss.dtsi> file for kernel while content
of header file should be copied to display.h for LK.
