#!/usr/bin/env bash
#
# rerun-rogo.sh - run rogomatic over and over again
#
# Copyright (c) 2026 by Landon Curt Noll.  All Rights Reserved.
#
# Permission to use, copy, modify, and distribute this software and
# its documentation for any purpose and without fee is hereby granted,
# provided that the above copyright, this permission notice and text
# this comment, and the disclaimer below appear in all of the following:
#
#       supporting documentation
#       source copies
#       source works derived from this source
#       binaries derived from this source or from derived source
#
# LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
# EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
# USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
#
# chongo (Landon Curt Noll) /\oo/\
#
# http://www.isthe.com/chongo/index.html
# https://github.com/lcn2
#
# Share and enjoy!  :-)


# setup
#
export VERSION="1.0 2026-04-16"
NAME=$(basename "$0")
export NAME
#
export V_FLAG=0
#
export NOOP=
export DO_NOT_PROCESS=
#
RUN_ROGO_TOOL=$(type -P run-rogo)
export RUN_ROGO_TOOL
if [[ -z $RUN_ROGO_TOOL ]]; then
    RUN_ROGO_TOOL="./run-rogo.sh"
fi
#
ROGOMATIC_TOOL=$(type -P rogomatic)
export ROGOMATIC_TOOL
if [[ -z $ROGOMATIC_TOOL ]]; then
    ROGOMATIC_TOOL="./rogomatic"
fi
#
PLAYER_TOOL=$(type -P player)
export PLAYER_TOOL
if [[ -z $PLAYER_TOOL ]]; then
    PLAYER_TOOL="./player"
fi
#
ROGUE_TOOL=$(type -P rogue)
export ROGUE_TOOL
if [[ -z $ROGUE_TOOL ]]; then
    ROGUE_TOOL="./rogue"
fi
#
export RGMDIR="/var/tmp/rogomatic"
export IDLE_SEC="20"


# usage
#
export USAGE="usage: $0 [-h] [-v level] [-V] [-n] [-N] [-R run-rogo] [-r rogomatic] [-P player] [-f rogue] [-D rgmdir] [-i idlesec]

    -h          print help message and exit
    -v level    set verbosity level (def level: $V_FLAG)
    -V          print version string and exit

    -n		go thru the actions, but do not update any files (def: do the action)
    -N          do not process anything, just parse arguments (def: process something)

    -R run-rogo		path to the run-rogo tool (def: $RUN_ROGO_TOOL)
    -r rogomatic	path to rogomatic (def: $ROGOMATIC_TOOL)
    -P player		path to player (def: $PLAYER_TOOL)
    -f rogue		path to rogue (def: $ROGUE_TOOL)
    -D rmdir		rogomatic directory (def: $RGMDIR)
    -i idlesec		seconds to check for an idle rogomatic (def: $IDLE_SEC)

Exit codes:
     0         all OK
     2         -h and help string printed or -V and version string printed
     3         command line error
     5	       some internal tool is not found or not an executable file
     6         problems found with or in the rogomatic directory
 >= 10         internal error

$NAME version: $VERSION"


# parse command line
#
while getopts :hv:VnNR:r:P:f:D:i: flag; do
  case "$flag" in
    h) echo "$USAGE"
	exit 2
	;;
    v) V_FLAG="$OPTARG"
	;;
    V) echo "$VERSION"
	exit 2
	;;
    n) NOOP="-n"
	;;
    N) DO_NOT_PROCESS="-N"
	;;
    R) RUN_ROGO_TOOL="$OPTARG"
        ;;
    r) ROGOMATIC_TOOL="$OPTARG"
        ;;
    P) PLAYER_TOOL="$OPTARG"
        ;;
    f) ROGUE_TOOL="$OPTARG"
        ;;
    D) RGMDIR="$OPTARG"
        ;;
    i) IDLE_SEC="$OPTARG"
        ;;
    \?) echo "$0: ERROR: invalid option: -$OPTARG" 1>&2
	echo 1>&2
	echo "$USAGE" 1>&2
	exit 3
	;;
    :) echo "$0: ERROR: option -$OPTARG requires an argument" 1>&2
	echo 1>&2
	echo "$USAGE" 1>&2
	exit 3
	;;
    *) echo "$0: ERROR: unexpected value from getopts: $flag" 1>&2
	echo 1>&2
	echo "$USAGE" 1>&2
	exit 3
	;;
  esac
done
if [[ $V_FLAG -ge 1 ]]; then
    echo "$0: debug[1]: debug level: $V_FLAG" 1>&2
fi
#
# remove the options
#
shift $(( OPTIND - 1 ));
#
# verify arg count
#
if [[ $# -ne 0 ]]; then
    echo "$0: ERROR: expected 0 args, found: $#" 1>&2
    echo "$USAGE" 1>&2
    exit 3
fi


# verify that the run-rogo tool is executable
#
if [[ ! -e $RUN_ROGO_TOOL ]]; then
    echo  "$0: ERROR: run-rogo does not exist: $RUN_ROGO_TOOL" 1>&2
    exit 5
fi
if [[ ! -f $RUN_ROGO_TOOL ]]; then
    echo  "$0: ERROR: run-rogo is not a regular file: $RUN_ROGO_TOOL" 1>&2
    exit 5
fi
if [[ ! -x $RUN_ROGO_TOOL ]]; then
    echo  "$0: ERROR: run-rogo is not an executable file: $RUN_ROGO_TOOL" 1>&2
    exit 5
fi


# verify that the rogomatic tool is executable
#
if [[ ! -e $ROGOMATIC_TOOL ]]; then
    echo  "$0: ERROR: rogomatic does not exist: $ROGOMATIC_TOOL" 1>&2
    exit 5
fi
if [[ ! -f $ROGOMATIC_TOOL ]]; then
    echo  "$0: ERROR: rogomatic is not a regular file: $ROGOMATIC_TOOL" 1>&2
    exit 5
fi
if [[ ! -x $ROGOMATIC_TOOL ]]; then
    echo  "$0: ERROR: rogomatic is not an executable file: $ROGOMATIC_TOOL" 1>&2
    exit 5
fi


# verify that the player tool is executable
#
if [[ ! -e $PLAYER_TOOL ]]; then
    echo  "$0: ERROR: player does not exist: $PLAYER_TOOL" 1>&2
    exit 5
fi
if [[ ! -f $PLAYER_TOOL ]]; then
    echo  "$0: ERROR: player is not a regular file: $PLAYER_TOOL" 1>&2
    exit 5
fi
if [[ ! -x $PLAYER_TOOL ]]; then
    echo  "$0: ERROR: player is not an executable file: $PLAYER_TOOL" 1>&2
    exit 5
fi


# verify that the rogomatic tool is executable
#
if [[ ! -e $ROGUE_TOOL ]]; then
    echo  "$0: ERROR: rogue does not exist: $ROGUE_TOOL" 1>&2
    exit 5
fi
if [[ ! -f $ROGUE_TOOL ]]; then
    echo  "$0: ERROR: rogue is not a regular file: $ROGUE_TOOL" 1>&2
    exit 5
fi
if [[ ! -x $ROGUE_TOOL ]]; then
    echo  "$0: ERROR: rogue is not an executable file: $ROGUE_TOOL" 1>&2
    exit 5
fi


# verify the rogomatic directory
#
if [[ ! -e $RGMDIR ]]; then
    echo "$0: ERROR: non-existent rogomatic directory path: $RGMDIR" 1>&2
    exit 6
fi
if [[ ! -d $RGMDIR ]]; then
    echo "$0: ERROR: not a directory: $RGMDIR" 1>&2
    exit 6
fi
if [[ ! -w $RGMDIR ]]; then
    echo "$0: ERROR: not a writable directory: $RGMDIR" 1>&2
    exit 6
fi


# print running info if verbose
#
# If -v 3 or higher, print exported variables in order that they were exported.
#
if [[ $V_FLAG -ge 3 ]]; then
    echo "$0: debug[3]: VERSION=$VERSION" 1>&2
    echo "$0: debug[3]: NAME=$NAME" 1>&2
    echo "$0: debug[3]: V_FLAG=$V_FLAG" 1>&2
    echo "$0: debug[3]: NOOP=$NOOP" 1>&2
    echo "$0: debug[3]: DO_NOT_PROCESS=$DO_NOT_PROCESS" 1>&2
    echo "$0: debug[3]: V_FLAG=$V_FLAG" 1>&2
    echo "$0: debug[3]: RUN_ROGO_TOOL=$RUN_ROGO_TOOL" 1>&2
    echo "$0: debug[3]: ROGOMATIC_TOOL=$ROGOMATIC_TOOL" 1>&2
    echo "$0: debug[3]: PLAYER_TOOL=$PLAYER_TOOL" 1>&2
    echo "$0: debug[3]: ROGUE_TOOL=$ROGUE_TOOL" 1>&2
    echo "$0: debug[3]: RGMDIR=$RGMDIR" 1>&2
    echo "$0: debug[3]: IDLE_SEC=$IDLE_SEC" 1>&2
fi


# -N stops early before any processing is performed
#
if [[ -n $DO_NOT_PROCESS ]]; then
    if [[ $V_FLAG -ge 3 ]]; then
	echo "$0: debug[3]: arguments parsed, -N given, exiting 0" 1>&2
    fi
    exit 0
fi


# run the run-rogo tool
#
if [[ -z $NOOP ]]; then
    while :; do
	if [[ $V_FLAG -ge 1 ]]; then
	    echo "$0: debug[1]: about to run: $RUN_ROGO_TOOL -r $ROGOMATIC_TOOL -P $PLAYER_TOOL -f $ROGUE_TOOL -D $RGMDIR" 1>&2
	fi
	"$RUN_ROGO_TOOL" -r "$ROGOMATIC_TOOL" -P "$PLAYER_TOOL" -f "$ROGUE_TOOL" -D "$RGMDIR"
    done
else
    echo "$0: debug[1]: due to use of -n, there is nothing to do" 1>&2
fi


# All Done!!! -- Jessica Noll, Age 2
#
exit 0
