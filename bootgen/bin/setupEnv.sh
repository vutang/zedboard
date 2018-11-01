#!/bin/bash
#
# COPYRIGHT NOTICE
# Copyright 1986-2014 Xilinx, Inc. All Rights Reserved.
#

#
# Locate the PATH found in FILE
#
# Usage: 
#  PATH = LOADPATH FILE BINROOT APPROOT
#
#  FILE     - File to read paths from
#  BINROOT  - current BINROOT location
#  APPROOT  - current APPROOT location
#  PATH     - Will be set to the path found in FILE is it exists 
#
# WARNING: The variables _BASELINE, _BASELINE_LINE, _BINROOT, _APPROOT,
#          and _path cannot be used elsewhere in loader
LOADPATH() {
  _BASELINE=$1
  _BINROOT=$2
  _APPROOT=$3
  _path=""
  _BASELINE_ERROR=""

  if [ -f "$_BASELINE" ]; then
    IFS=$'\r\n'
    while read _BASELINE_LINE || [ -n "$_BASELINE_LINE" ]; do
      if [ "$_BASELINE_LINE" = "Not Defined" ]; then
        _BASELINE_ERROR="ERROR: A baseline has not been defined."
      else
        if [ "${_BASELINE_LINE:0:2}" = ".." ]; then
          _BASELINE_LINE="$_APPROOT/$_BASELINE_LINE"
        fi
        if [ -d "$_BASELINE_LINE" ]; then
          cd $_BASELINE_LINE
          _path="$PWD"
          cd - > /dev/null 2>&1
        else
          _BASELINE_ERROR="ERROR: The baseline '$_BASELINE_LINE' does not exist."
        fi
      fi
    done < $_BASELINE
    IFS=$' \t\n'
  fi

  if [ ! -z "$_BASELINE_ERROR" ]; then
    _path="$_BASELINE_ERROR\n"
    if [ -d "$_APPROOT/data/patches" ]; then
      _path="$_path A baseline is the path of the full install being patched and is required\n"
      _path="$_path for patches to function correctly.\n\n"
      _path="$_path To establish a baseline for this patch, run 'establish-baseline.sh'\n"
      _path="$_path in '$_APPROOT/scripts/patch'\n"
      _path="$_path and enter the necessary install path when prompted.\n\n"
      _path="$_path If you received this error running from a non-patch installation,\n"
      _path="$_path please contact customer support."
    else
      _path="$_path Vivado is unable to locate the path to PlanAhead. PlanAhead is necessary\n"
      _path="$_path for Vivado to run. Please contact customer support for assistance."
    fi
    echo $_path
    return 1
  fi

  echo $_path
  return 0
}

#
# Locate the DIRS found in PATH
#
# Usage: 
#  DIRS = FINDDIRS PATH
#
#  PATH     - Location to find folders in
#  DIRS     - Will be set to the folders found in PATH if it exists 
#
# WARNING: The variables _PATH and _dirs cannot be used elsewhere in loader
FINDDIRS() {
  _PATH=$1
  _dirs=""

  if [ -d "$_PATH" ]; then
    for _dir in `ls -1v "$_PATH"`; do
      if [ -z "$_dirs" ]; then
        _dirs=$_PATH/$_dir/vivado
      else
        _dirs=$_dirs:$_PATH/$_dir/vivado
      fi
    done
  fi

  echo $_dirs
  return 0
}

#
# Configure RDI_APPROOT which is used for the bases of all other 
# PATH variables
#
RDI_BINROOT=`dirname "$0"`
cd "$RDI_BINROOT"
RDI_BINROOT=`pwd`
cd - > /dev/null 2>&1
RDI_APPROOT=`dirname "$RDI_BINROOT"`

if [ "$XIL_PA_NO_XILINX_OVERRIDE" != "1" ]; then
  unset MYXILINX
fi

RDI_PATCHROOT=""
_BASELINE_SEARCH=true
#Do While loop to search for nested patch areas.
while $_BASELINE_SEARCH ; do
  _BASELINE_FILE=$RDI_APPROOT/data/baseline.txt
  RDI_BASELINE=$(LOADPATH "$_BASELINE_FILE" "$RDI_BINROOT" "$RDI_APPROOT")
  if [ "$?" = "1" ] ; then
      echo -e $RDI_BASELINE
      exit 1
  fi

  if [ "$XIL_PA_NO_XILINX_OVERRIDE" != "1" ]; then
      if [ -z "$MYXILINX" ]; then
          # locate ISE baseline to set MYXILINX
          _BASELINE_FILE=$RDI_APPROOT/data/ise_baseline.txt
          _TMP_MYXILINX=$(LOADPATH "$_BASELINE_FILE" "$RDI_BINROOT" "$RDI_APPROOT")
          if [ "$?" = "1" ] ; then
              echo -e $_TMP_MYXILINX
              exit 1
          fi
          if [ -d "$_TMP_MYXILINX" ]; then
              MYXILINX=$_TMP_MYXILINX
              export MYXILINX
          fi
      fi
  fi

  if [ ! -z "$RDI_BASELINE" ]; then
      if [ -d "$RDI_BASELINE" ]; then
          # This is a reverse ordered list
          _RDI_PATCHDIRS=$(FINDDIRS "$RDI_APPROOT/patches")
          if [ ! -z $_RDI_PATCHDIRS ]; then
              RDI_PATCHROOT=$_RDI_PATCHDIRS:$RDI_PATCHROOT
              export RDI_PATCHROOT
          fi
          RDI_PATCHROOT="$RDI_APPROOT:$RDI_PATCHROOT"
          RDI_APPROOT=$RDI_BASELINE
          RDI_BASELINE=""
      else
          _BASELINE_SEARCH=false
      fi
  else
      _BASELINE_SEARCH=false      
  fi
done

RDI_BASEROOT=`dirname "$RDI_APPROOT"`
export RDI_BINROOT RDI_APPROOT RDI_BASEROOT

if [ "`uname -o`" != "Cygwin" ]; then
    # Append (reverse ordered) RDI_PATCHROOT with valid locations specified by MYVIVADO
    if [ ! -z "$MYVIVADO" ]; then
        _RDI_EXTENDED_WARNING=
        IFS=$':'
        for SUB_PATCH in $MYVIVADO; do
            if [ -d "$SUB_PATCH" ]; then
                _RDI_PATCHROOT="$SUB_PATCH:$_RDI_PATCHROOT"
            else
                if [ "$SUB_PATCH" != "" ]; then
                    echo "WARNING: Ignoring invalid MYVIVADO location $SUB_PATCH."
                   _RDI_EXTENDED_WARNING=1
                fi
            fi
        done
        IFS=$' \t\n'
        if [ ! -z $_RDI_EXTENDED_WARNING ]; then
            echo "Resolution: An invalid MYVIVADO location has been detected. To resolve this issue:"
            echo ""
            echo "1. Verify the value of MYVIVADO is accurate by viewing the value the variable via 'set MYVIVADO' for Windows or 'echo \$MYVIVADO' for Linux, and update it as needed."
            echo ""
            echo "2. To unset the variable using on Windows using 'set MYVIVADO=' or remove it from Advanced System Settings\Environment Variables. On Linux 'unsetenv MYVIVADO'"
            echo ""
        fi
        _RDI_EXTENDED_WARNING=
        RDI_PATCHROOT=$RDI_PATCHROOT$_RDI_PATCHROOT
    fi
fi
export RDI_PATCHROOT

# HDI_APPROOT is needed for backwards compatibility
# until code is changed to refer to RDI_APPROOT.
HDI_APPROOT=$RDI_APPROOT
export HDI_APPROOT

_RDI_PATCHDIRS=$(FINDDIRS "$RDI_APPROOT/patches")
if [ ! -z "$_RDI_PATCHDIRS" ]; then
    RDI_PATCHROOT=$_RDI_PATCHDIRS:$RDI_PATCHROOT
    export RDI_PATCHROOT
fi

_RDI_SETENV_RUN=true
export _RDI_SETENV_RUN
