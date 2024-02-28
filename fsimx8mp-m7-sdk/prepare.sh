#!/bin/bash

# This is the base path of the FreeRTOS BPS
PACKAGE_PATH=${PWD}
# Define the board names supported by the FreeRTOS BSP here
declare -a SUPPORTED_BOARDS=("efusa9x" "picocoma9x" "picocoremx6sx" "picocoremx7ulp" "picocoremx8mm"
                "picocoremx8mp")
declare -a SUPPORTED_SOCS=("fsimx6sx" "fsimx6sx" "fsimx6sx" "fsimx7ulp" "fsimx8mm" "fsimx8mp")

if [ ${#SUPPORTED_BOARDS[@]} -ne ${#SUPPORTED_SOCS[@]} ]; then
	printf "SUPPORTED_BOARDS and SUPPORTED_SOCS have to have the same length with matching board and socket\n"
	exit -1
fi

# Array with -D flags for CMake
declare -a CMAKE_DEFINES=()

# defaults
default_ram_size=1024
default_build_type='r'
default_workaround_flag='n'

# Font codes go here
BOLD='\033[1m'
NORMAL='\033[0m'
LIGHT_GREEN='\033[1;32m'
LIGHT_RED='\033[0;31m'

################################################################################
# function: createLinks()
#
# parameters: $1: board name
#
# return: -
#
# description:
# Create symlinks to the board specific files determined
# by $1
#
################################################################################
function createLinks()
{
  for target in $(ls . | grep $1); do
    local LINK_NAME=${PROJECT_PATH}/$(ls $target | cut -d '_' -f1 --complement)
    # Always unlink first if link exists
    if [ -e $LINK_NAME ]; then
      unlink $LINK_NAME
    fi
    ln -sT $PWD/$target $LINK_NAME
    # error detection
    if  [ $? -ne 0 ]; then
      printf "An error occured. Not all symlinks could be created!\b\n"
      exit
    fi
  done

  printf "Symlinks to board specific files created!\b\n"
}

################################################################################
# function: setBoardSize()
#
# parameters: $1: board size
#
# return: -
#
# description:
# Set the correct board size and vring size for the rpmsg
# examples
#
################################################################################
function setBoardSize()
{
  if [ $1 -eq 512 ]; then
    CMAKE_DEFINES+=('-DVRING0_BASE:STRING=0x9FFF0000' '-DVRING1_BASE:STRING=0x9FFF8000')
	cd devices/MCIMX6X/linker/gcc/
	ln -s -f MCIMX6X_M4_ddr_512MB.ld MCIMX6X_M4_ddr.ld
  elif [ $1 -ge 1024 ]; then
    CMAKE_DEFINES+=('-DVRING0_BASE:STRING=0xBFFF0000' '-DVRING1_BASE:STRING=0xBFFF8000')
	cd devices/MCIMX6X/linker/gcc/
	ln -s -f MCIMX6X_M4_ddr_1GB.ld MCIMX6X_M4_ddr.ldd
  else
    printf "Incorrect board size.\b\n"
    exit -1
  fi
  cd -
}

function setVringBuffers()
{
	v1="-DVRING0_BASE:STRING=${1}"
	num1=${1}
	num2=0x8000
	printf -v NEWBASE '%#x' "$((num1 + num2))"
	v2="-DVRING1_BASE:STRING=${NEWBASE}"

    CMAKE_DEFINES+=(${v1} ${v2})
}

################################################################################
# function: setBuildType()
#
# parameters: $1: build type as character
#
# return: -
#
# description:
# Set the build type to release or debug
#
################################################################################
function setBuildType()
{
  type
  if [[ "$1" ==  "d" ]]; then
    CMAKE_DEFINES+=('-DCMAKE_BUILD_TYPE=Debug')
  elif [[ "$1" == "r" ]]; then
    CMAKE_DEFINES+=('-DCMAKE_BUILD_TYPE=Release')
  else
    printf "Incorrect build type.\b\n"
    exit -1
  fi
}

################################################################################
# function: setWorkaround()
#
# parameters: $1: flag indicating if workaround is needed
#
# return: -
#
# description:
# Set the define for the RDC workaround for the PicoCOMA9X
#
################################################################################
function setWorkaround()
{
  if [[ "$1" == "y" ]]; then
    CMAKE_DEFINES+=('-DWORKAROUND=1')
  elif [[ "$1" == "n" ]]; then
    CMAKE_DEFINES+=('-DWORKAROUND=0')
  else
    printf "Wrong input given!\b\n"
    exit -1
  fi
}

#################################### Main #######################################

# Board selection
printf "Choose on of the following boards for which you want to build the examples:\b\n"

count=0
for board in ${SUPPORTED_BOARDS[@]}; do
  count=$((count+1))
  # Format: board_name[number]
  printf "%s[${BOLD}%s${NORMAL}]\t" "$board" "$count"
done

printf "\nEnter number in []-brackets for the corresponding board: "
read board_given
FOUND=false

# Only numerical input allowed!
if [ -n $board_given ] && [[ $board_given =~ ^[0-9]+$ ]]; then
  # Check input for range of valid entries
  if [ $board_given -le ${#SUPPORTED_BOARDS[@]} ]; then
        # Check PCB version for compilation
	if [ "${SUPPORTED_SOCS[$board_given-1]}" = "fsimx8mm" ]; then
                printf "\nPlease choose the production variant V3/V4 (LPDDR4) [1] V5/V6 (DDR3L) [2]: "
                read pcb_version

                if ! [[ $pcb_version =~ ^[1-2]+$ ]]; then
                        printf "\r\nPlease select ${LIGHT_RED}1${NORMAL} or ${LIGHT_RED}2${NORMAL}\n"
                        exit -1
                fi
        fi

	# Path to the example folder for the F&S boards
	chosen_board=${SUPPORTED_BOARDS[$board_given-1]}
 	chosen_soc=${SUPPORTED_SOCS[$board_given-1]}
	PROJECT_PATH="$PACKAGE_PATH/examples/${chosen_soc}"
	if [ "${SUPPORTED_SOCS[$board_given-1]}" = "fsimx6sx" ]; then
		cd $PROJECT_PATH/board_specific_files/${chosen_board}
		createLinks ${chosen_board}
		cd $PACKAGE_PATH
	fi
  else
    printf "You gave ${LIGHT_RED}%s${NORMAL}, but there are only ${LIGHT_GREEN}%s${NORMAL} boards!\b\n" "$board_given" "${#SUPPORTED_BOARDS[@]}"
    exit -1
  fi
else
  echo "No numerical input given!"
  exit -1
fi

# Build type
printf "Do you want a ${BOLD}Release${Normal} or ${BOLD}Debug${NORMAL} build?\b\n"
printf "${BOLD}(r/d)${NORMAL} [default: $default_build_type]: "
read build_type

if [ -n "$build_type" ]; then
  setBuildType $build_type
else
  setBuildType $default_build_type
fi

if [ "$chosen_soc" = "fsimx6sx" ]; then
	#VRING BUFFERS imx6sx
	default_vring=0x8fff0000
	default_reserved="n"
	printf "Did you change the address of the reserved DRAM-memory?\r\n"
	printf "${BOLD}(y/n)${NORMAL} [default: $default_reserved]: "
	read yn
	if [ -z "$yn" ]; then
 		yn=$default_reserved
	fi
	case $yn in
       [Yy]* ) echo "New end address the reserved memory (in HEX)?";
			   read new_vring_end
			   if [[ $new_vring_end != "0x"* ]]; then
					new_vring_end="0x"$new_vring_end
			   fi
			   new_vring_base=$new_vring_end; vring_size=0x10000;
			   printf -v new_vring_base '%#x' "$((new_vring_end - vring_size))";
			   setVringBuffers $new_vring_base;
			   echo "Setting vring_buffers to $new_vring_base";;
       * ) setVringBuffers $default_vring;
			   echo "Setting vring_buffers to $default_vring";;
	esac
else
	#VRING BUFFERS imx7ulp
	setVringBuffers 0x9ff00000
fi

# Workaround for faulty PicoCOMA9X/PicoCoreMX6SX
if [ "${chosen_board}" = "picocoma9x" ] || [ "${chosen_board}" = "picocoremx6sx" ]; then
  printf "If you have a PicoCOMA9X or a PicoCoreMX6SX with a fused PCIE_DISABLE, you need\b\n"
  printf "the workaround to disable RDC_* calls, otherwise your CPU will hang\b\n"
  printf "(See documentation, last chapter)\b\n"
  printf "${BOLD}(y/n)${NORMAL} [default: $default_workaround_flag]: "
  read workaround_flag
fi

if [ -n "$workaround_flag" ]; then
  setWorkaround $workaround_flag
else
  setWorkaround $default_workaround_flag
fi

printf "${LIGHT_GREEN}All set up, starting cmake...${NORMAL}\b\n\n"
# Generate Makefile
cmake -DCMAKE_TOOLCHAIN_FILE="tools/cmake_toolchain_files/armgcc.cmake" -G "Unix Makefiles" -DPCB_VERSION:STRING=${pcb_version} -DBOARD:STRING=${chosen_board} -DSOC:STRING=${chosen_soc} ${CMAKE_DEFINES[@]:0} .
# Change default target to install
sed -i.bak '/default\_/s/all/install/g' Makefile && rm Makefile.bak
