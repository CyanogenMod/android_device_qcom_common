# Copyright (c) 2013, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#     * Neither the name of The Linux Foundation nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# parser.pl
use XML::LibXML;
use IO::Handle;
use warnings;
use strict;

# initialize the parser
my $parser = new XML::LibXML;

# open a filehandle and parse
my $fh = new IO::Handle;

my $name = "";
my $nameinH = "";
my $Version = "";
my $xmldoc = $parser->parse_file($ARGV[0]);
if(scalar @ARGV < 2)
{
	print "2 command line arguments required.\n";
	print "1:XML Document to parse\n";
	print "2:[Panel|Platform]\n";
	print "Example: perl parse.pl Panel.xml Panel\n";
	exit 1;
}

if(uc($ARGV[1]) eq "PANEL")
{
	for my $property($xmldoc->findnodes('/GCDB/Version'))
	{
		$Version = $property->textContent();
	}

	for my $property($xmldoc->findnodes('/GCDB/PanelId'))
	{
		$name = $property->textContent();
	}

	open (my $PANELDTSI, '>dsi-panel-' . $name . '.dtsi');

	for my $property($xmldoc->findnodes('/GCDB/PanelH'))
	{
		$nameinH = $property->textContent();
	}
	open (my $PANELH, '>panel_' . $nameinH . '.h');
	printheader($PANELH);
	print $PANELH "\n#ifndef _PANEL_". uc($nameinH) . "_H_\n";
	print $PANELH "\n#define _PANEL_". uc($nameinH) . "_H_\n";
	printdtsheader($PANELDTSI, $nameinH);


	for my $property($xmldoc->findnodes('/GCDB/PanelEntry/PanelName'))
	{
		print $PANELDTSI "\t\tqcom,mdss-dsi-panel-name = "
				. $property->textContent() . ";\n";
	}
	printSectionHeader($PANELH, "HEADER files");

	print $PANELH "#include \"panel.h\"\n\n";

	printSectionHeader($PANELH, "Panel configuration");
	print $PANELH "\n";

	for my $property($xmldoc->findnodes('/GCDB/PanelEntry/PanelController'))
	{
		(my $controllerName) = $property->textContent() =~ /"([^"]*)"/;
		print $PANELDTSI "\t\tqcom,mdss-dsi-panel-controller = <&"
						. $controllerName . ">;\n";
	}

	for my $property($xmldoc->findnodes('/GCDB/PanelEntry'))
	{
		my @attrs = ("PanelType");
		foreach(@attrs)
		{
			push(my @tmp, $_);
			printPanelType($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-panel-type");
		}
		for my $dest($xmldoc->findnodes('/GCDB/PanelEntry/PanelDestination'))
		{
			print $PANELDTSI "\t\tqcom,mdss-dsi-panel-destination = "
					. lc($dest->textContent()) . ";\n";
		}
		@attrs = ("PanelWithEnableGPIO", "BitClockFrequency",
		"PanelClockrate", "PanelFrameRate", "DSIVirtualChannelId",
		"DSIInitMasterTime", "DSIStream",
		"PanelWidth", "PanelHeight", "HFrontPorch",
		"HBackPorch", "HPulseWidth", "HSyncSkew", "VBackPorch",
		"VFrontPorch", "VPulseWidth", "HLeftBorder", "HRightBorder",
		"VTopBorder", "VBottomBorder", "HActiveRes", "VActiveRes",
		"ColorFormat", "ColorOrder", "UnderFlowColor", "BorderColor",
		"PanelOperatingMode", "PixelPacking", "PixelAlignment");

		foreach(@attrs)
		{
			push(my @tmp, $_);
			my $val = $_;
			$val =~ s/DSI//g;
			$val = convertLower($val);


			if($_ eq "ColorFormat")
			{
				printArray($PANELDTSI, \@tmp, $property,
							"qcom,mdss-dsi-bpp");
			}
			elsif($_ eq "PanelWithEnableGPIO")
			{
				printArray($PANELDTSI, \@tmp, $property,
							"qcom,mdss-dsi-panel-with-enable-gpio");
			}
			elsif($_ eq "UnderFlowColor")
			{
				printArray($PANELDTSI, \@tmp, $property,
							"qcom,mdss-dsi-underflow-color");
			}
			elsif($_ eq "PanelFrameRate")
			{
				printArray($PANELDTSI, \@tmp, $property,
							"qcom,mdss-dsi-panel-framerate");
			}
			else
			{
				printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-" . $val);
			}
		}

		my @panelConfigAttrs = ("PanelController",
		"PanelCompatible", "PanelInterface", "PanelType",
		"PanelDestination", "PanelOrientation", "PanelClockrate",
		"PanelFrameRate", "PanelChannelId", "DSIVirtualChannelId",
		"PanelBroadcastMode", "DSILP11AtInit", "DSIInitMasterTime",
		"DSIStream", "InterleaveMode","BitClockFrequency",
		"PanelOperatingMode", "PanelWithEnableGPIO");

		printDataStruct($PANELH, \@panelConfigAttrs, $property,
				"panel_config", lc($nameinH) . "_panel_data",
				"qcom,mdss_dsi_" .lc($nameinH));
	}

	print $PANELH "\n\n";
	printSectionHeader($PANELH, "Panel resolution");

	for my $property($xmldoc->findnodes('/GCDB/PanelEntry'))
	{
		my @panelConfigAttrs = ("PanelWidth", "PanelHeight",
		"HFrontPorch", "HBackPorch", "HPulseWidth", "HSyncSkew",
		"VFrontPorch", "VBackPorch", "VPulseWidth", "HLeftBorder",
		"HRightBorder", "VTopBorder", "VBottomBorder", "HActiveRes",
		"VActiveRes", "InvertDataPolarity", "InvertVsyncPolarity",
		"InvertHsyncPolarity");

		printStruct($PANELH, \@panelConfigAttrs, $property,
			"panel_resolution", lc($nameinH) . "_panel_res");
		print $PANELH "\n";
	}

	print $PANELH "\n";
	printSectionHeader($PANELH, "Panel Color Information" );

	for my $property($xmldoc->findnodes('/GCDB/PanelEntry'))
	{
		my @panelConfigAttrs = ("ColorFormat", "ColorOrder",
		"UnderFlowColor", "BorderColor", "PixelPacking",
		"PixelAlignment");

		printStruct($PANELH, \@panelConfigAttrs, $property,
			"color_info", lc($nameinH) . "_color");
		print $PANELH "\n";
	}

	print $PANELH "\n";
	printSectionHeader($PANELH, "Panel Command information" );

	for my $property ($xmldoc->findnodes('/GCDB/PanelEntry/OnCommand'))
	{
		printOnCommand($PANELH, $property, lc($nameinH) .
				"_on_command", lc($nameinH) . "_on_cmd");
		print $PANELH "\n\n";
		printHexArray($PANELDTSI, $property, "qcom,mdss-dsi-on-command");
	}
	for my $property ($xmldoc->findnodes('/GCDB/PanelEntry/OffCommand'))
	{
		printOnCommand($PANELH, $property, lc($nameinH) .
			"_off_command", lc($nameinH) . "off_cmd");
		print $PANELH "\n\n";
		printHexArray($PANELDTSI, $property, "qcom,mdss-dsi-off-command");
	}

	for my $property($xmldoc->findnodes('/GCDB/PanelEntry'))
	{
		my @commandAttrs = ("OnCommandState", "OffCommandState");
		printStruct($PANELH, \@commandAttrs, $property,
				"command_state", lc($nameinH) . "_state");
		print $PANELH "\n\n";

		foreach(@commandAttrs)
		{
			push(my @tmp, $_);
			my $lower = convertLower($_);
			printCommandState($PANELDTSI, \@tmp, $property,
						"qcom,mdss-dsi-" . $lower);
		}

		my @attrs = ("HSyncPulse", "TrafficMode",
		"DSILaneMap",);

		foreach(@attrs)
		{
			push(my @tmp, $_);
			my $lower = $_;
			$lower =~ s/DSI//g;
			$lower = convertLower($lower);
			printArray($PANELDTSI, \@tmp, $property,
						"qcom,mdss-dsi-" . $lower);
		}

		@attrs = ("BLLPEOFPowerMode");
		foreach(@attrs)
		{
			push(my @tmp, $_);
			printBoolean($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-bllp-eof-power-mode");
		}

		@attrs = ("BLLPPowerMode");
		foreach(@attrs)
		{
			push(my @tmp, $_);
			printBoolean($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-bllp-power-mode");
		}

		@attrs = ("PanelBroadcastMode", "HFPPowerMode", "HBPPowerMode",
		"HSAPowerMode",
		"Lane0State", "Lane1State", "Lane2State", "Lane3State");

		foreach(@attrs)
		{
			push(my @tmp, $_);
			my $lower = $_;
			$lower =~ s/DSI//g;
			if(index($_, "PowerMode") != -1)
			{
				substr($lower, 0, index($_, "PowerMode")) = lc(substr($lower, 0, index($_, "PowerMode")));
			}
			elsif(index($lower, "Lane") != -1)
			{
				$lower = substr($lower, 0, 4) . "-" . substr($lower, 4, 1) . "-" . substr($lower, 5, 5);
				$lower = lc($lower);
			}
			else
			{
				$lower = convertLower($lower);
			}
			printBoolean($PANELDTSI, \@tmp, $property,
				"qcom,mdss-dsi-" . $lower);
		}
	}

	printSectionHeader($PANELH, "Command mode panel information");
	print $PANELH "\n";
	for my $property($xmldoc->findnodes('/GCDB/PanelEntry'))
	{
		my @attrs = ("TECheckEnable", "TEPinSelect", "TEUsingTEPin",
		"AutoRefreshEnable", "AutoRefreshFrameNumDiv",
		"TEvSyncRdPtrIrqLine", "TEvSyncContinuesLines",
		"TEvSyncStartLineDivisor", "TEPercentVariance", "TEDCSCommand",
		"DisableEoTAfterHSXfer", "CmdModeIdleTime");

		printStruct($PANELH, \@attrs, $property, "commandpanel_info",
					lc($nameinH) . "_command_panel");
		print $PANELH "\n\n";

		@attrs = ("TEPinSelect", "AutoRefreshEnable", "AutoRefreshFrameNumDiv",
		"TEvSyncRdPtrIrqLine", "TEvSyncContinuesLines",
		"TEvSyncStartLineDivisor", "TEPercentVariance", "TEDCSCommand",
		"DisableEoTAfterHSXfer", "CmdModeIdleTime");

		foreach(@attrs)
		{
			push(my @tmp, $_);
			if($_ eq "TEPinSelect")
			{
				printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-te-pin-select");
			}
			elsif($_ eq "TEvSyncRdPtrIrqLine")
			{
				printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-te-v-sync-rd-ptr-irq-line");
			}
			elsif($_ eq "TEvSyncContinuesLines")
			{
				printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-te-v-sync-continues-lines");
			}
			elsif($_ eq "TEvSyncStartLineDivisor")
			{
				printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-te-v-sync-start-line-divisor");
			}
			elsif($_ eq "TEPercentVariance")
			{
				printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-te-percent-variance");
			}
			elsif($_ eq "TEDCSCommand")
			{
				printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-te-dcs-command");
			}
			else
			{
				printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-" . convertLower($_));
			}
		}

		@attrs = ("TECheckEnable", "TEUsingTEPin");
		foreach(@attrs)
		{
			if($_ eq "TECheckEnable")
			{
				push(my @tmp, $_);
				printBoolean($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-te-check-enable");
			}
			elsif($_ eq "TEUsingTEPin")
			{
				push(my @tmp, $_);
				printBoolean($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-te-using-te-pin");
			}
		}

	}

	printSectionHeader($PANELH, "Video mode panel information");
	print $PANELH "\n";

	for my $property($xmldoc->findnodes('/GCDB/PanelEntry'))
	{
		my @commandAttrs = ("HSyncPulse", "HFPPowerMode", "HBPPowerMode",
		"HSAPowerMode", "BLLPEOFPowerMode", "BLLPPowerMode",
		"TrafficMode", "DMADelayAfterVsync", "BLLPEOFPower");

		printStruct($PANELH, \@commandAttrs, $property,
				"videopanel_info", lc($nameinH) . "_video_panel");
		print $PANELH "\n\n";
	}

	printSectionHeader($PANELH, "Lane Configuration");
	print $PANELH "\n";

	for my $property($xmldoc->findnodes('/GCDB/PanelEntry'))
	{
		my @commandAttrs = ("DSILanes", "DSILaneMap", "Lane0State",
		"Lane1State", "Lane2State", "Lane3State");

		printStruct($PANELH, \@commandAttrs, $property,
			"lane_configuration", lc($nameinH) . "_lane_config");
		print $PANELH "\n\n";

	}
	print $PANELH "\n";
	printSectionHeader($PANELH, "Panel Timing");

	for my $property ($xmldoc->findnodes('/GCDB/PanelEntry/PanelTimings'))
	{
		printTargetPhysical($PANELH, $property, lc($nameinH) .
			"_timings","uint32_t");
		print $PANELH "\n\n";
		printHexArray($PANELDTSI, $property,
						"qcom,mdss-dsi-panel-timings");
	}

	for my $property ($xmldoc->findnodes('/GCDB/PanelEntry/PanelRotation'))
	{
		printOnCommand($PANELH, $property, lc($nameinH) .
				"_rotation", lc($nameinH) . "_rotation_cmd");
		print $PANELH "\n\n";
		printHexArray($PANELDTSI, $property,
						"qcom,mdss-dsi-panelrotation");
	}

	for my $property($xmldoc->findnodes('/GCDB/PanelEntry'))
	{
		my @timingAttrs = ("DSIMDPTrigger", "DSIDMATrigger",
		"TClkPost", "TClkPre");

		printStruct($PANELH, \@timingAttrs, $property, "panel_timing",
						lc($nameinH) . "_timing_info");
		print $PANELH "\n\n";
	}
	for my $tmpProperty($xmldoc->findnodes('/GCDB/PanelEntry/ResetSequence'))
	{
		print $PANELH "static struct panel_reset_sequence " .
				lc($nameinH) . "_reset_seq = {\n ";
		printResetSeqinH($PANELH, $tmpProperty);
		print $PANELH "\n\n"
	}
	printSectionHeader($PANELH, "Backlight Settings");
	print $PANELH "\n";
	for my $property($xmldoc->findnodes('/GCDB/PanelEntry'))
	{
		my @timingAttrs = ("BLInterfaceType", "BLMinLevel",
		"BLMaxLevel", "BLStep", "BLPMICControlType", "BLPMICModel");

		printStruct($PANELH, \@timingAttrs, $property, "backlight",
						lc($nameinH) . "_backlight");
		print $PANELH "\n\n";

		my @attrs = ("TClkPost", "TClkPre");

		foreach(@attrs)
		{
			push(my @tmp, $_);
			my $lower = $_;
			$lower =~ s/DSI//g;
			$lower = convertLower($lower);
			printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-" . $lower);
		}

		@attrs = ("BLMinLevel");
		foreach(@attrs)
		{
			push(my @tmp, $_);
			printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-bl-min-level");
		}
		@attrs = ("BLMaxLevel");
		foreach(@attrs)
		{
			push(my @tmp, $_);
			printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-bl-max-level");
		}
		@attrs = ("BLPMICPWMFrequency");
		foreach(@attrs)
		{
			push(my @tmp, $_);
			printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-bl-pmic-pwm-frequency");
		}
		@attrs = ("BLPMICBankSelect");
		foreach(@attrs)
		{
			push(my @tmp, $_);
			printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-bl-pmic-bank-select");
		}
		@attrs = ("DSIDMATrigger");
		foreach(@attrs)
		{
			push(my @tmp, $_);
			printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-dma-trigger");
		}
		@attrs = ("DSIMDPTrigger");
		foreach(@attrs)
		{
			push(my @tmp, $_);
			printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-mdp-trigger");
		}

		@attrs = ("BLPMICControlType");
		foreach(@attrs)
		{
			push(my @tmp, $_);
			printControlType($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-bl-pmic-control-type");
		}

		@attrs = ("PanPhysicalWidthDimension",
			"PanPhysicalHeightDimension");
		foreach(@attrs)
		{
			push(my @tmp, $_);
			my $lower = $_;
			$lower = convertLower($lower);
			printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-" . $lower);
		}

		my @attr = ("FBCbpp", "FBCPacking",
		"FBCBias",
		"FBCHLineBudget", "FBCBlockBudget",
		"FBCLosslessThreshold", "FBCLossyThreshold", "FBCRGBThreshold",
		"FBCLossyModeIdx");

		foreach(@attr)
		{
			push(my @tmp, $_);
			my $lower = convertLower($_);
			printArray($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-" . $lower);
		}

		@attr = ("FBCEnable", "FBCQuantError", "FBCPATMode", "FBCVLCMode",
		"FBCBFLCMode");
		foreach(@attr)
		{
			push(my @tmp, $_);
			my $lower = convertLower($_);
			printBoolean($PANELDTSI, \@tmp, $property,
					"qcom,mdss-dsi-" . $lower);
		}
		for my $tmpProperty ($xmldoc->findnodes('/GCDB/PanelEntry/ResetSequence'))
		{
			print $PANELDTSI "\t";
			printResetSeq($PANELDTSI, $tmpProperty,
					"qcom,mdss-dsi-reset-sequence");
			print $PANELDTSI "\n";
		}
	}

	print $PANELH "\n#endif /*_PANEL_" . uc($nameinH) . "_H_*/\n";
	print $PANELDTSI "\t};\n};\n";
	close ($PANELH);
	close ($PANELDTSI);
}
elsif(uc($ARGV[1]) eq "PLATFORM")
{
	for my $property($xmldoc->findnodes('/GCDB/Version'))
	{
		$Version = $property->textContent();
	}

	for my $property ($xmldoc->findnodes('/GCDB/PlatformEntry/PlatformId')){
		($name) = $property->textContent() =~ /"([^"]*)"/;
	}

	open (my $PLATFORMDTSI, '>platform-' . $name . '.dtsi');
	open (my $PLATFORMH, '>platform_' . $name . '.h');
	print $PLATFORMDTSI "/*\n " . $name . " target DTSI file.\n*/\n{\n";
	print $PLATFORMDTSI "\tqcom,platform = \"" . $name . "\";\n";

	printheader($PLATFORMH);
	print $PLATFORMH "\n#ifdef _PLATFORM_" . uc($name) . "_H_\n";

	printSectionHeader($PLATFORMH, "HEADER files");
	print $PLATFORMH "#include <display_resource.h>\n\n";
	printSectionHeader($PLATFORMH, "GPIO configuration");

	my @gpioConfAttrs = ("PinSource", "PinId", "PinStrength",
				"PinDirection",	"Pull", "PinState");

	for my $property ($xmldoc->findnodes('/GCDB/PlatformEntry/ResetGPIO'))
	{
		printStruct($PLATFORMH, \@gpioConfAttrs, $property, "gpio_pin",
								"reset_gpio");
		print $PLATFORMH "\n\n";

		printGPIO($PLATFORMDTSI, $property, "qcom,platform-reset-gpio");
	}
	for my $property ($xmldoc->findnodes('/GCDB/PlatformEntry/EnableGPIO'))
	{
		printStruct($PLATFORMH, \@gpioConfAttrs, $property, "gpio_pin",
								"enable_gpio");
		print $PLATFORMH "\n\n";

		printGPIO($PLATFORMDTSI, $property, "qcom,platform-enable-gpio");
	}
	for my $property ($xmldoc->findnodes('/GCDB/PlatformEntry/TEGPIO'))
	{
		printStruct($PLATFORMH, \@gpioConfAttrs, $property, "gpio_pin",
								 "te_gpio");
		print $PLATFORMH "\n\n";

		printGPIO($PLATFORMDTSI, $property, "qcom,platform-te-gpio");
	}
	for my $property ($xmldoc->findnodes('/GCDB/PlatformEntry/PWMGPIO'))
	{
		printStruct($PLATFORMH, \@gpioConfAttrs, $property, "gpio_pin",
								"pwm_gpio");
		print $PLATFORMH "\n\n";

		printGPIO($PLATFORMDTSI, $property, "qcom,platform-pwm-gpio");
	}
	printSectionHeader($PLATFORMH, "Supply configuration");
	print $PLATFORMH "static struct ldo_entry ldo_entry_array[] = {\n  ";
	my @ldoEntryAttrs = ("SupplyName", "SupplyMinVoltage", "SupplyMaxVoltage",
			"SupplyEnableLoad", "SupplyDisableLoad", "SupplyPreOnSleep",
			"SupplyPostOnSleep", "SupplyPreOffSleep", "SupplyPostOffSleep");
	my @ldoEntryAttrsinH = ("SupplyName", "SupplyId", "SupplyType", "SupplyMaxVoltage",
			"SupplyEnableLoad", "SupplyDisableLoad","SupplyPreOnSleep",
			"SupplyPostOnSleep", "SupplyPreOffSleep", "SupplyPostOffSleep");
	my $ldocounter = 0;

	for (my $i = 1; $i <= 5; $i++)
	{
		for my $property ($xmldoc->findnodes('/GCDB/PlatformEntry/SupplyEntry' . $i))
		{
			printSupplyinH($PLATFORMH, \@ldoEntryAttrsinH, $property);
			$ldocounter++;

			printSupplyEntry($PLATFORMDTSI, \@ldoEntryAttrs, $property,
						"qcom,platform-supply-entry" . $i);
		}
	}

	print $PLATFORMH "};\n\n";
	print $PLATFORMH "#define TOTAL_LDO_DEFINED " . $ldocounter . "\n\n";

	printSectionHeader($PLATFORMH, "Target Physical configuration");
	print $PLATFORMH "\n";
	print $PLATFORMDTSI "\n";

	for my $property ($xmldoc->findnodes('/GCDB/PlatformEntry/PanelStrengthCtrl'))
	{
		printTargetPhysical($PLATFORMH, $property, "panel_strength_ctrl", "uint32_t");
		print $PLATFORMH "\n\n";

		printHexArray($PLATFORMDTSI, $property, "qcom,platform-strength-ctrl");
	}

	for my $property ($xmldoc->findnodes('/GCDB/PlatformEntry/PanelBISTCtrl'))
	{
		printTargetPhysical($PLATFORMH, $property, "panel_bist_ctrl", "char");
		print $PLATFORMH "\n\n";

		printHexArray($PLATFORMDTSI, $property, "qcom,platform-bist-ctrl");
	}

	for my $property ($xmldoc->findnodes('/GCDB/PlatformEntry/PanelRegulatorSettings'))
	{
		printTargetPhysical($PLATFORMH, $property, "panel_regulator_settings", "uint32_t");
		print $PLATFORMH "\n\n";

		printHexArray($PLATFORMDTSI, $property, "qcom,platform-regulator-settings");
	}

	for my $property ($xmldoc->findnodes('/GCDB/PlatformEntry/PanelLaneConfig'))
	{
		printTargetPhysical($PLATFORMH, $property, "panel_lane_config", "char");
		print $PLATFORMH "\n\n";

		printHexArray($PLATFORMDTSI, $property, "qcom,platform-lane-config");
	}

	for my $property ($xmldoc->findnodes('/GCDB/PlatformEntry/PanelPhysicalCtrl'))
	{
		printTargetPhysical($PLATFORMH, $property, "panel_physical_ctrl", "uint32_t");
		print $PLATFORMH "\n\n";

		printHexArray($PLATFORMDTSI, $property, "qcom,platform-physical-ctrl");
	}

	printSectionHeader($PLATFORMH, "Other Configuration");
	print $PLATFORMH "\n";
	print $PLATFORMDTSI "\n";
	for my $property ($xmldoc->findnodes('/GCDB/PlatformEntry/ContinuousSplash'))
	{
		my $splash = $property->textContent();
		print $PLATFORMH "#define " . $name . "_CONTINUOUS_SPLASH " . $splash . "\n\n";

		my @attrs = ("ContinuousSplash");
		printArray($PLATFORMDTSI, \@attrs, $xmldoc->findnodes('/GCDB/PlatformEntry'),
					"qcom,platform-continuous-splash");
	}
	for my $property ($xmldoc->findnodes('/GCDB/PlatformEntry/DSIFeatureEnable'))
	{
		my $dsi = $property->textContent();
		print $PLATFORMH "#define " . $name . "_DSI_FEATURE_ENABLE " . $dsi . "\n";

		my @attrs = ("DSIFeatureEnable");
		printArray($PLATFORMDTSI, \@attrs, $xmldoc->findnodes('/GCDB/PlatformEntry'),
					"qcom,platform-dsi-feature-enable");
	}

	print $PLATFORMH "\n#endif /*_PLATFORM_" . uc($name) . "_H_*/\n";
	print $PLATFORMDTSI "};\n";

	close ($PLATFORMH);
	close ($PLATFORMDTSI);
}


sub printHexArray
{
	my $fh = shift;
	my $property = shift;
	my $name = shift;
	(my $element) = $property->textContent() =~ /"([^"]*)"/;
	$element =~ s/,//g;
	$element =~ s/0x//g;
	print $fh "\t\t" . $name . " = [" . $element . "];\n";

}

sub printTargetPhysical
{
	my $fh = shift;
	my $property = shift;
	my $name = shift;
	my $datatype = shift;
	(my $element) = $property->textContent() =~ /"([^"]*)"/;
	print $fh "static const " . $datatype . " " . $name . "[] = {\n";
	$element =~ s/\t//g;
	$element =~ s/^/  /mg;
	print $fh $element . "\n};";
}

sub printOnCommand
{
	my $fh = shift;
	my $property = shift;
	my $name = shift;
	my $cmdname = shift;
	(my $element) = $property->textContent() =~ /"([^"]*)"/;
	my @lines = split /\n/, $element;
	my $toPrint = "";
	my $i = 0;
	foreach my $line (@lines)
	{
		my @sep = split /,/, $line;
		print $fh "static char " . $cmdname . $i . "[] = {\n";
		$toPrint = "";
		if(scalar @sep > 7)
		{
			my $cmdlen = $sep[6];
			my $cmdtype = $sep[0];
			$cmdtype =~ s/ //g;
			$cmdtype =~ s/\t//g;
			$cmdlen =~ s/ //g;
			$cmdlen =~ s/\t//g;
			if($cmdtype eq "0x29" || $cmdtype eq "0x39")
			{
				$toPrint .=  $cmdlen . ", 0x00, " . $cmdtype . ", 0xC0,\n";
			}
			my $j = 0;
			for(my $i = 7; $i < scalar @sep; $i++)
			{
				my $tmp = $sep[$i];
				$tmp =~ s/ //g;
				$tmp =~ s/\t//g;
				if($tmp ne "")
				{
					$toPrint  .= $tmp . ", ";
					$j++;
				}
				if (($j % 4) == 0)
				{
					chop($toPrint);
					$toPrint .= "\n";
				}
			}
			if($cmdtype eq "0x29" || $cmdtype eq "0x39")
			{
				for( ; ($j % 4) ne 0 ; $j++)
				{
					$toPrint .= "0xFF, ";
				}
			}
			else
			{
				$toPrint .= $cmdtype . ", 0x80";
			}
			$toPrint .= " };\n"
		}
		$i++;

		$toPrint .= "\n\n";
		print $fh $toPrint ;
	}

	print $fh "\n\nstatic struct mipi_dsi_cmd " . $name . "[] = {\n";
	my $bool = 0;
	$toPrint = "";
	$i = 0;

	foreach my $line (@lines)
	{
		my @sep = split /,/, $line;

		if(scalar @sep > 7)
		{
			my $cmdtype = $sep[0];
			$cmdtype =~ s/ //g;
			$cmdtype =~ s/\t//g;

			my $cmdsize = 0;
			my $hexsize = 0;

			if($cmdtype eq "0x29" || $cmdtype eq "0x39")
			{
			   my $j = 0;
			   for(my $i = 7; $i < scalar @sep; $i++)
			   {
				 my $tmp = $sep[$i];
				 $tmp =~ s/ //g;
				 $tmp =~ s/\t//g;
				 if($tmp ne "")
				 {
					$cmdsize += 1;
					$j++;
				 }
			    }
			    for( ; ($j % 4) ne 0 ; $j++)
			    {
				  $cmdsize += 1;
			    }

			    # calculate the correct size of command
			    $hexsize = sprintf("{ 0x%x , ", $cmdsize + 4);
			}
			else
			{
				$hexsize = sprintf("{ 0x%x , ", 4);
			}


			$toPrint .=  $hexsize;

			$toPrint .= $cmdname . $i . "},";

			$i++;

		}

		$toPrint .= "\n";
	}
	chop($toPrint);
	chop($toPrint);
	$toPrint .= "\n";
	print $fh $toPrint . "};";


	print $fh "\n#define " . uc($name) . " " . $i . "\n";
}

sub printSupplyPower
{
	my $fh = shift;
	my $property = shift;
	my $toPrint = "";
	$toPrint .= "{ ";
	for (my $i = 1; $i <=5; $i++)
	{
		for my $element ($property->findnodes("SupplyEntryId" . $i))
		{
			$toPrint .= $element->textContent() . ", ";
		}
	}
	$toPrint .= "}, { ";
	for (my $i = 1; $i <=5; $i++)
	{
		for my $element ($property->findnodes("Sleep" . $i))
		{
			$toPrint .= $element->textContent() . ", ";
		}
	}
	$toPrint .= "}\n};";
	print $fh $toPrint;
}

sub printResetSeqinH
{
	my $fh = shift;
	my $property = shift;
	print $fh "{ ";
	for (my $i = 1; $i <=5; $i++)
	{
		for my $element ($property->findnodes("PinState" . $i))
		{
			print $fh $element->textContent() . ", ";
		}
	}
	print $fh "}, { ";
	for (my $i = 1; $i <=5; $i++)
	{
		for my $element ($property->findnodes("PulseWidth" . $i))
		{
			print $fh $element->textContent() . ", ";
		}
	}
	print $fh "}, ";
	for my $element ($property->findnodes("EnableBit"))
	{
		print $fh $element->textContent();
		print $fh "\n};\n";
	}
}

sub printResetSeq
{
	my $fh = shift;
	my $property = shift;
	my $name = shift;
	print $fh "\t" . $name . " = ";
	my $first = 1;
	for (my $i = 1; $i <=5; $i++)
	{
		for my $element ($property->findnodes("./PinState" . $i))
		{
			if($first == 1)
			{
				$first = 0;
			}
			else
			{
				print $fh ", ";
			}
			print $fh "<" . $element->textContent();
		}
		for my $element ($property->findnodes("./PulseWidth" . $i))
		{
			print $fh " ";
			print $fh $element->textContent();
			print $fh ">";
		}
	}
	print $fh ";";
}

sub printSupplyinH
{
	my $fh = shift;
	my $attrs = shift;
	my @attrs = @$attrs;
	my $property = shift;
	my $flag = 0;
	print $fh "{ ";
	foreach (@attrs) {
		my $node = "./" . $_;
		my $found = 0;
		for my $element ($property->findnodes($node))
		{
			$found = 1;
			if($flag == 1)
			{
				print $fh ", ";
			}
			else
			{
				$flag = 1;
			}
			print $fh $element->textContent();
		}
		if($found == 0)
		{
			if($flag == 1)
			{
				print $fh ", ";
			}
			else
			{
				$flag = 1;
			}
			print $fh 0;
		}
	}
	print $fh "},\n";
}

sub convertLower
{
	my $input = shift;
	for (my $i = 1; $i < length($input); $i++) {
		if (ord(substr($input, $i, 1)) <= ord('Z'))
		{
			substr($input, $i, 1) = lc(substr($input, $i, 1));
			$input = substr($input, 0, $i) . '-' . substr($input, $i, length($input));
		}
	}
	return lc($input);
}

sub printSupplyEntry
{
	my $fh = shift;
	my $attrs = shift;
	my @attrs = @$attrs;
	my $property = shift;
	my $name = shift;
	print $fh "\t" . $name . " {\n";
	foreach(@attrs)
	{
		for my $element ($property->findnodes("./" . $_))
		{
			if ($element->textContent() =~ /\D/)
			{
				print $fh "\t\tqcom," . convertLower($_) . " = " . $element->textContent() . ";\n";
			}
			else
			{
				print $fh "\t\tqcom," . convertLower($_) . " = <" . $element->textContent() . ">;\n";
			}
		}
	}
	print $fh "\t};\n\n";
}

sub printArray
{
	my $fh = shift;
	my $attrs = shift;
	my @attrs = @$attrs;
	my $property = shift;
	my $name = shift;
	my $first = 1;
	foreach(@attrs)
	{
		for my $element ($property->findnodes("./" . $_))
		{
			print $fh "\t\t" . $name . " = <";
			if($first == 1)
			{
				$first = 0;
			}
			else
			{
				print $fh " ";
			}
			print $fh $element->textContent() . ">;\n";
		}
	}
}

sub printControlType
{
	my $fh = shift;
	my $attrs = shift;
	my @attrs = @$attrs;
	my $property = shift;
	my $name = shift;
	my $first = 1;
	foreach(@attrs)
	{
		for my $element ($property->findnodes("./" . $_))
		{
			print $fh "\t\t" . $name . " = \"";
			if($element->textContent() eq "0")
			{
				print $fh "bl_ctrl_pwm";
			}
			elsif($element->textContent() eq "1")
			{
				print $fh "bl_ctrl_wled";
			}
			elsif($element->textContent() eq "2")
			{
				print $fh "bl_ctrl_dcs";
			}
			print $fh "\";\n";
		}
	}
}

sub printCommandState
{
	my $fh = shift;
	my $attrs = shift;
	my @attrs = @$attrs;
	my $property = shift;
	my $name = shift;
	my $first = 1;
	foreach(@attrs)
	{
		for my $element ($property->findnodes("./" . $_))
		{
			print $fh "\t\t" . $name . " = \"";
			if($element->textContent() eq "0")
			{
				print $fh "dsi_lp_mode";
			}
			else
			{
				print $fh "dsi_hs_mode";
			}
			print $fh "\";\n";
		}
	}
}

sub printPanelType
{
	my $fh = shift;
	my $attrs = shift;
	my @attrs = @$attrs;
	my $property = shift;
	my $name = shift;
	my $first = 1;
	foreach(@attrs)
	{
		for my $element ($property->findnodes("./" . $_))
		{
			print $fh "\t\t" . $name . " = \"";
			if($element->textContent() eq "0")
			{
				print $fh "dsi_video_mode";
			}
			else
			{
				print $fh "dsi_cmd_mode";
			}
			print $fh "\";\n";
		}
	}
}

sub printBoolean
{
	my $fh = shift;
	my $attrs = shift;
	my @attrs = @$attrs;
	my $property = shift;
	my $name = shift;
	my $first = 1;
	foreach(@attrs)
	{
		for my $element ($property->findnodes("./" . $_))
		{
			if($element->textContent() eq "1")
			{
				print $fh "\t\t" . $name ;
				print $fh ";\n";
			}
		}
	}
}

sub printGPIO
{
	my $fh = shift;
	my $property = shift;
	my $name = shift;
	print $fh "\t" . $name . " = <";
	for my $element ($property->findnodes("./PinSource"))
	{
		print $fh "&";
		print $fh $element->textContent() =~ /"([^"]*)"/;
		print $fh " ";
	}
	for my $element ($property->findnodes("./PinId"))
	{
		print $fh $element->textContent() . " ";
	}
	for my $element ($property->findnodes("./PinState"))
	{
		print $fh $element->textContent();
	}
	print $fh ">;\n";
}


sub printStruct
{
	my $fh = shift;
	my $attrs = shift;
	my @attrs = @$attrs;
	my $property = shift;
	my $structName = shift;
	my $name = shift;
	print $fh "static struct " . $structName . " " . $name . " = {\n  ";
	my $flag = 0;
	foreach (@attrs) {
		my $node = "./" . $_;
		my $found = 0;
		for my $element ($property->findnodes($node))
		{
			$found = 1;
			if($flag == 1)
			{
				print $fh ", ";
			}
			else
			{
				$flag = 1;
			}
			print $fh $element->textContent();
		}
		if($found == 0)
		{
			if($flag == 1)
			{
				print $fh ", ";
			}
			else
			{
				$flag = 1;
			}
			print $fh 0;
		}
	}
	print $fh "\n};"
}

sub printDataStruct
{
	my $fh = shift;
	my $attrs = shift;
	my @attrs = @$attrs;
	my $property = shift;
	my $structName = shift;
	my $name = shift;
	my $nameinDTS = shift;
	print $fh "static struct " . $structName . " " . $name . " = {\n  ";
	my $flag = 0;
	my $firsttime = 1;
	print $fh "\"" . $nameinDTS . "\", ";
	for my $element ($property->findnodes("./PanelController"))
	{
		my $controller = $element->textContent();
		if ($controller eq "\"mdss_dsi1\"")
		{
			print $fh "\"dsi:1:\", ";
		}
		else
		{
			print $fh "\"dsi:0:\", ";
		}
	}
	shift(@attrs);
	foreach (@attrs) {
		my $node = "./" . $_;
		my $found = 0;
		for my $element ($property->findnodes($node))
		{
			$found = 1;
			if($flag == 1)
			{
				print $fh ", ";
			}
			else
			{
				$flag = 1;
			}
			print $fh $element->textContent();
		}
		if($found == 0)
		{
			if($flag == 1)
			{
				print $fh ", ";
			}
			else
			{
				$flag = 1;
			}
			print $fh 0;
		}
		if ($firsttime == 1)
		{
			print $fh ",\n  ";
			$flag = 0;
		}
		$firsttime = 0;
	}
	print $fh "\n};"
}

sub printSectionHeader
{
	my $fh = shift;
	my $section = shift;
	print $fh "/*---------------------------------------------------------------------------*/\n";
	print $fh "/* " . $section;
	for (my $i = length($section); $i <= 73; $i++) {
		print $fh " ";
	}
	print $fh "*/\n";
	print $fh "/*---------------------------------------------------------------------------*/\n";
}

sub printheader
{
	my $fh = shift;
	print $fh "/* Copyright (c) 2013, The Linux Foundation. All rights reserved.\n";
	print $fh " *\n";
	print $fh " * Redistribution and use in source and binary forms, with or without\n";
	print $fh " * modification, are permitted provided that the following conditions\n";
	print $fh " * are met:\n";
	print $fh " *  * Redistributions of source code must retain the above copyright\n";
	print $fh " *    notice, this list of conditions and the following disclaimer.\n";
	print $fh " *  * Redistributions in binary form must reproduce the above copyright\n";
	print $fh " *    notice, this list of conditions and the following disclaimer in\n";
	print $fh " *    the documentation and/or other materials provided with the\n";
	print $fh " *    distribution.\n";
	print $fh " *  * Neither the name of The Linux Foundation nor the names of its\n";
	print $fh " *    contributors may be used to endorse or promote products derived\n";
	print $fh " *    from this software without specific prior written permission.\n";
	print $fh " *\n";
	print $fh " * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n";
	print $fh " * \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n";
	print $fh " * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS\n";
	print $fh " * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE\n";
	print $fh " * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,\n";
	print $fh " * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,\n";
	print $fh " * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS\n";
	print $fh " * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED\n";
	print $fh " * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,\n";
	print $fh " * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT\n";
	print $fh " * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n";
	print $fh " * SUCH DAMAGE.\n";
	print $fh " */\n";
	print $fh "\n";
	print $fh "/*---------------------------------------------------------------------------\n";
	print $fh " * This file is autogenerated file using gcdb parser. Please do not edit it.\n";
	print $fh " * Update input XML file to add a new entry or update variable in this file\n";
	print $fh " * VERSION = " . $Version . "\n";
	print $fh " *---------------------------------------------------------------------------*/\n";
}

sub printdtsheader
{
	my $fh = shift;
	my $name = shift;

	print $fh "/* Copyright (c) 2013, The Linux Foundation. All rights reserved.\n";
	print $fh " *\n";
	print $fh " * This program is free software; you can redistribute it and/or modify\n";
	print $fh " * it under the terms of the GNU General Public License version 2 and\n";
	print $fh " * only version 2 as published by the Free Software Foundation.\n";
	print $fh " *\n";
	print $fh " * This program is distributed in the hope that it will be useful,\n";
	print $fh " * but WITHOUT ANY WARRANTY; without even the implied warranty of\n";
	print $fh " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n";
	print $fh " * GNU General Public License for more details.\n";
	print $fh " */\n\n";
	print $fh "/*---------------------------------------------------------------------------\n";
	print $fh " * This file is autogenerated file using gcdb parser. Please do not edit it.\n";
	print $fh " * Update input XML file to add a new entry or update variable in this file\n";
	print $fh " * VERSION = " . $Version . "\n";
	print $fh " *---------------------------------------------------------------------------*/\n";

	print $fh "&mdss_mdp {\n";
	print $fh "\tdsi_" . lc($name) . ": qcom,mdss_dsi_" . lc($name) . " {\n";
}
