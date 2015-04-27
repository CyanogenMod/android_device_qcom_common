# Copyright (c) 2011, The Linux Foundation. All rights reserved.
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
# list_clks.sh
# Echoes all clock values inside debugfs/clk
# Author: Jack Cheung

# This script is intended to also work with adb shell

verbosity=0
root=/debug
choose_clk="all"

# Parse arguments
while getopts 'v:d:' OPTION
do
  case $OPTION in
  v)  verbosity=$OPTARG
      ;;
  d)  root=$OPTARG
      ;;
  ?)  echo "Usage: list_all_clks.sh [-v <verbosity level>] [-d <path to debug>]"
      echo "Options:"
      echo "   -v,          Set verbosity level (default 0)"
      echo "                   0 - Display Local, Enabled, Rate"
      echo "                   1 - Display Measured rate"
      echo "                   2 - Display Supported rate"
      echo "   -d,          Set path to debugfs (default /debug)"
      exit
      ;;
  esac
done
shift $(($OPTIND - 1))

echo "Verbosity: $verbosity"
echo "Root Debug Dir: $root"
echo ""

case $verbosity in
0)  echo -e "                                Local       Enabled     Rate"
    ;;
1)  echo -e "                                Local       Enabled     Rate        Measured Rate"
    ;;
2)  echo -e "                                Local       Enabled     Rate        Measured Rate"
    ;;
esac

# Print table for local, enabled, rate, and measured rate
# Loop through every directory in root dir
for clk_dir in $root/clk/*
do
  # Reset values to null
  local_val=
  enable_val=
  rate_val=
  measure_val=

  # Leaves only basename
  clk_name=${clk_dir##*/}
  
  # Special case - Need to ignore the debug_suspend
  case $clk_name in
    debug_suspend) continue
    ;;
  esac
  
  # Read values and hide errors by outputting to /dev/null
  local_val=`cat $clk_dir/is_local 2>/dev/null`
  enable_val=`cat $clk_dir/enable 2>/dev/null`
  rate_val=`cat $clk_dir/rate 2>/dev/null`
  measure_val=`cat $clk_dir/measure 2>/dev/null`
  
  # Need to pad values with spaces so columns align correctly
  # Loop and remove last character untill it is correct length
  clk_name="$clk_name                                "
  while true
  do
    # Chops off last character
    clk_name=${clk_name%?}

    length=${#clk_name}
    case $length in
      32) break
          ;;
    esac
  done
  
  # local_val must be 0 or 1
  case $local_val in
    0) local_val="N           "
    ;;
    1) local_val="Y           "
    ;;
  esac
  
  # enable_val must be -1, 0, or 1
  # Still accounting for when enable_val is accidentally seen 
  # as MAX unsigned int instead of signed -1
  case $enable_val in
    -1) enable_val="NA          "
        ;;
    4294967295) enable_val="NA          "
                ;;
    0) enable_val="OFF         "
       ;;
    1) enable_val="ON          "
       ;;
  esac

  # Padding value with spaces, similar to clk_name
  rate_val="$rate_val            "
  while true
  do
    rate_val=${rate_val%?}
    length=${#rate_val}
    case $length in
      12) break
          ;;
    esac
  done
    
  case $verbosity in
  0) echo -e "$clk_name$local_val$enable_val$rate_val"
     ;;
  1) echo -e "$clk_name$local_val$enable_val$rate_val$measure_val"
     ;;
  2) echo -e "$clk_name$local_val$enable_val$rate_val$measure_val"
     ;;
  esac
done

echo ""

# Print supported rates separately. Too many to fit in table cleanly
case $verbosity in
2)  echo -n "Supported Rates"
    for clk_dir in $root/clk/*
    do
      myfile=
      myline=
      count=0
      clk_name=${clk_dir##*/}
  
      myfile=`cat $clk_dir/list_rates 2>/dev/null`
      for myline in $myfile
      do
        # Print 8 values per line
        case $count in
        0) echo ""
           echo ""
           echo "$clk_name"
           echo -n "$myline"
           ;;
        7) echo -n ", $myline"
           ;;
        8) echo ""
           echo -n "$myline"
           count=0
           ;;
        *) echo -n ", $myline"
           ;;
        esac
        count=$(($count + 1))
      done
    done
    ;;
esac
echo -e "\nDone"
