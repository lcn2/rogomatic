#!/usr/bin/env bash
#
# run-rogo.sh - run rogomatic
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
export VERSION="1.3.0 2026-07-07"
NAME=$(basename "$0")
export NAME
#
export V_FLAG=0
export SECS=""
#
export NOOP=
export DO_NOT_PROCESS=
#
if [[ -x ./rogomatic ]]; then
    ROGOMATIC_TOOL="./rogomatic"
else
    ROGOMATIC_TOOL=$(type -P rogomatic)
fi
export ROGOMATIC_TOOL
#
if [[ -x ./player ]]; then
    PLAYER_TOOL="./player"
else
    PLAYER_TOOL=$(type -P player)
fi
export PLAYER_TOOL
#
if [[ -x ./rogue ]]; then
    ROGUE_TOOL="./rogue"
elif [[ -x ../rogue5.4/rogue ]]; then
    ROGUE_TOOL="../rogue5.4/rogue"
else
    ROGUE_TOOL=$(type -P rogue)
fi
export ROGUE_TOOL
export SEED=""


# NOTE: The following RGMDIR is NOT the default for rogomatic (/var/tmp/rogomatic)
#       This means you can run rogue(6) and rogomatic by hand
#       while a rogomatic rerun loop is running without interference.
#
export RGMDIR="/var/tmp/rogo"


# usage
#
export USAGE="usage: $0 [-h] [-v level] [-V] [-n] [-N] [-r rogomatic] [-P player] [-f rogue] [-D rgmdir] [-S seed]

    -h          print help message and exit
    -v level    set verbosity level (def level: $V_FLAG)
    -V          print version string and exit

    -n		go thru the actions, but do not update any files (def: do the action)
    -N          do not process anything, just parse arguments (def: process something)

    -a secs		set the timeout timer to secs seconds (def: no timeout timer)
    -r rogomatic	path to rogomatic (def: $ROGOMATIC_TOOL)
    -P player		path to player (def: $PLAYER_TOOL)
    -f rogue		path to rogue (def: $ROGUE_TOOL)
    -D rmdir		rogomatic directory (def: $RGMDIR)
    -S seed		set rogomatic seed (def: use a random seed)

Exit codes:
     0         all OK
     1         player already running
     2         -h and help string printed or -V and version string printed
     3         command line error
     5	       some internal tool is not found or not an executable file
     6         problems found with or in the rogomatic directory
 >= 10         internal error

$NAME version: $VERSION"


# parse command line
#
while getopts :hv:Va:nNr:P:f:D:S: flag; do
  case "$flag" in
    h) echo "$USAGE"
	exit 2
	;;
    v) V_FLAG="$OPTARG"
	;;
    V) echo "$VERSION"
	exit 2
	;;
    a) SECS="$OPTARG"
	;;
    n) NOOP="-n"
	;;
    N) DO_NOT_PROCESS="-N"
	;;
    r) ROGOMATIC_TOOL="$OPTARG"
        ;;
    P) PLAYER_TOOL="$OPTARG"
        ;;
    f) ROGUE_TOOL="$OPTARG"
        ;;
    D) RGMDIR="$OPTARG"
        ;;
    S) SEED="$OPTARG"
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
if [[ ! -d $RGMDIR ]]; then
    mkdir -p "$RGMDIR"
fi
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
    echo "$0: debug[3]: SECS=$SECS" 1>&2
    echo "$0: debug[3]: NOOP=$NOOP" 1>&2
    echo "$0: debug[3]: DO_NOT_PROCESS=$DO_NOT_PROCESS" 1>&2
    echo "$0: debug[3]: V_FLAG=$V_FLAG" 1>&2
    echo "$0: debug[3]: ROGOMATIC_TOOL=$ROGOMATIC_TOOL" 1>&2
    echo "$0: debug[3]: PLAYER_TOOL=$PLAYER_TOOL" 1>&2
    echo "$0: debug[3]: ROGUE_TOOL=$ROGUE_TOOL" 1>&2
    echo "$0: debug[3]: RGMDIR=$RGMDIR" 1>&2
    echo "$0: debug[3]: SEED=$SEED" 1>&2
fi


# -N stops early before any processing is performed
#
if [[ -n $DO_NOT_PROCESS ]]; then
    if [[ $V_FLAG -ge 3 ]]; then
	echo "$0: debug[3]: arguments parsed, -N given, exiting 0" 1>&2
    fi
    exit 0
fi


# verify that player isn't already running
#
flock -n -E 1 -o "$RGMDIR/player.lck" true
status="$?"
if [[ $status -eq 1 ]]; then
    echo "$0: ERROR: player appears to be running, file is locked: $RGMDIR/player.lck" 1>&2
    exit 1
elif [[ $status -ne 0 ]]; then
    echo "$0: ERROR: flock -n -E 1 -o $RGMDIR/player.lck failed, error: $status" 1>&2
    exit 10
fi


# exec the rogomatic code
#
if [[ -z $SEED ]]; then
    if [[ -z $SECS ]]; then
	if [[ $V_FLAG -ge 1 ]]; then
	    echo "$0: debug[1]: about to run: exec $ROGOMATIC_TOOL -P $PLAYER_TOOL -f $ROGUE_TOOL -D $RGMDIR" 1>&2
	fi
	if [[ -z $NOOP ]]; then
	    exec "$ROGOMATIC_TOOL" -P "$PLAYER_TOOL" -f "$ROGUE_TOOL" -D "$RGMDIR"
	fi
    else
	if [[ $V_FLAG -ge 1 ]]; then
	    echo "$0: debug[1]: about to run: exec $ROGOMATIC_TOOL -P $PLAYER_TOOL -f $ROGUE_TOOL -D $RGMDIR -a $SECS" 1>&2
	fi
	if [[ -z $NOOP ]]; then
	    exec "$ROGOMATIC_TOOL" -P "$PLAYER_TOOL" -f "$ROGUE_TOOL" -D "$RGMDIR" -a "$SECS"
	fi
    fi
else
    if [[ -z $SECS ]]; then
	if [[ $V_FLAG -ge 1 ]]; then
	    echo "$0: debug[1]: about to run: exec $ROGOMATIC_TOOL -P $PLAYER_TOOL -f $ROGUE_TOOL -D $RGMDIR -S $SEED" 1>&2
	fi
	if [[ -z $NOOP ]]; then
	    exec "$ROGOMATIC_TOOL" -P "$PLAYER_TOOL" -f "$ROGUE_TOOL" -D "$RGMDIR" -S "$SEED"
	fi
    else
	if [[ $V_FLAG -ge 1 ]]; then
	    echo "$0: debug[1]: about to run: exec $ROGOMATIC_TOOL -P $PLAYER_TOOL -f $ROGUE_TOOL -D $RGMDIR -S $SEED -a $SECS" 1>&2
	fi
	if [[ -z $NOOP ]]; then
	    exec "$ROGOMATIC_TOOL" -P "$PLAYER_TOOL" -f "$ROGUE_TOOL" -D "$RGMDIR" -S "$SEED" -a "$SECS"
	fi
    fi
fi


# All Done!!! -- Jessica Noll, Age 2
#
exit 0
