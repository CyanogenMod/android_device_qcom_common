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
