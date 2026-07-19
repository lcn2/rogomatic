#!/usr/bin/env bash
#
# run_rogo.sh - run rogomatic
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
export VERSION="1.4.2 2026-07-19"
NAME=$(basename "$0")
export NAME
#
export V_FLAG=0
export SECS=
#
export NOOP=
export DO_NOT_PROCESS=
#
ROGOMATIC_TOOL=$(type -P rogomatic)
if [[ -x ./rogomatic ]]; then
    ROGOMATIC_TOOL="./rogomatic"
fi
export ROGOMATIC_TOOL
#
PLAYER_TOOL=$(type -P player)
if [[ -x ./player ]]; then
    PLAYER_TOOL="./player"
fi
export PLAYER_TOOL
#
ROGUE_TOOL=$(type -P rogue)
if [[ -x ../rogue5.4/rogue ]]; then
    ROGUE_TOOL="../rogue5.4/rogue"
elif [[ -x ./rogue ]]; then
    ROGUE_TOOL="./rogue"
fi
export ROGUE_TOOL
#
export SEED=
export GOODGAME=20
export USLEEP=14000
export CAP_H_FLAG=
export CAP_G_FLAG=
export CAP_U_FLAG=
export D_FLAG=
export E_FLAG=


# NOTE: The following RGMDIR is NOT the default for rogomatic (/var/tmp/rogomatic)
#       This means you can run rogue(6) and rogomatic by hand
#       while a rogomatic rerun loop is running without interference.
#
export RGMDIR="/var/tmp/rogo"


# options we will to the rogue(6) command line
#
unset OPTION
declare -ag OPTION


# usage
#
export USAGE="usage: $0
        [-h] [-v level] [-V] [-n] [-N]
        [-a secs] [-d] [-D rgmdir] [-e] [-f rogue] [-G goodlvl] [-H]
        [-P player] [-r rogomatic] [-S seed] [-U usec]

    -h          print help message and exit
    -v level    set verbosity level (def level: $V_FLAG)
    -V          print version string and exit
    -n          go thru the actions, but do not update any files (def: do the action)
    -N          do not process anything, just parse arguments (def: process something)

    -a secs             set the timeout timer to secs seconds (def: no timeout timer)
    -d                  use a UTC date and time sub-directory under rogomatic directory path (def: don't)
    -D rgmdir           rogomatic directory (def: $RGMDIR)
                            NOTE: if rgmdir is /var/tmp/rogomatic unstuck_player won't unstick unless -A is used
    -e                  turn off rogomatic game logging (def: rogomatic game log is $RGMDIR/gamelog)
    -f rogue            path to rogue (def: $ROGUE_TOOL)
    -G goodlvl          set the good game level to goodlvl (def: $GOODGAME)
    -H                  disable the so-called rogomatic halftime show (def: show it)

    -P player           path to player (def: $PLAYER_TOOL)
    -r rogomatic        path to rogomatic (def: $ROGOMATIC_TOOL)
    -S seed             set rogomatic seed (def: use a random seed)
    -U usec             set the sleep time between actions to usec microseconds (def: $USLEEP)
                            NOTE: <=0 ==> no delay and implies -H

Exit codes:
     0         all OK
     1         player already running
     2         -h and help string printed or -V and version string printed
     3         command line error
     5         some internal tool is not found or not an executable file
     6         problems found with or in the rogomatic directory
     7         rogomatic returned an error
 >= 10         internal error

$NAME version: $VERSION"


# parse command line
#
while getopts :hv:VnNa:dD:ef:G:HP:r:S:U: flag; do
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

    a) SECS="$OPTARG"
	;;
    d) D_FLAG="-d"
        ;;
    D) RGMDIR="$OPTARG"
        ;;
    e) E_FLAG="-e"
        ;;
    f) ROGUE_TOOL="$OPTARG"
        ;;
    G) GOODGAME="$OPTARG"
	CAP_G_FLAG="-G"
        ;;
    H) CAP_H_FLAG="-H"
        ;;

    P) PLAYER_TOOL="$OPTARG"
        ;;
    r) ROGOMATIC_TOOL="$OPTARG"
        ;;
    S) SEED="$OPTARG"
        ;;
    U) USLEEP="$OPTARG"
	CAP_U_FLAG="-U"
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
if [[ $USLEEP -lt 0 ]]; then
    echo "$0: ERROR: -U $USLEEP must be >= 0" 1>&2
    echo "$USAGE" 1>&2
    exit 3
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


# build rogue(6) command line options
#
if [[ -n $CAP_H_FLAG || $USLEEP -le 0 ]]; then
    OPTION+=("-H")	# no half time show
fi
if [[ -n $CAP_U_FLAG ]]; then
    OPTION+=("-U")		# usec delay (or none)
    OPTION+=("$USLEEP")
fi
OPTION+=("-P")		# set player path
OPTION+=("$PLAYER_TOOL")
OPTION+=("-f")		# set rogue path
OPTION+=("$ROGUE_TOOL")
OPTION+=("-D")		# set rogomatic directory path
OPTION+=("$RGMDIR")
if [[ -n $CAP_G_FLAG ]]; then
    OPTION+=("-G")		# set good game level
    OPTION+=("$GOODGAME")
fi
if [[ -n $SECS ]]; then
    OPTION+=("-a")		# set sleep time between actions
    OPTION+=("$SECS")
fi
if [[ -n $SEED ]]; then
    OPTION+=("-S")		# set seed for pseudo-random number generator
    OPTION+=("$SEED")
fi
if [[ -n $D_FLAG ]]; then
    OPTION+=("-d")		# use a UTC date and time sub-directory under rogomatic directory path
fi
if [[ -n $E_FLAG ]]; then
    OPTION+=("-e")		# turn OFF rogomatic game logging
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
    echo "$0: debug[3]: GOODGAME=$GOODGAME" 1>&2
    echo "$0: debug[3]: ROGUE_TOOL=$ROGUE_TOOL" 1>&2
    echo "$0: debug[3]: RGMDIR=$RGMDIR" 1>&2
    echo "$0: debug[3]: SEED=$SEED" 1>&2
    echo "$0: debug[3]: USLEEP=$USLEEP" 1>&2
    echo "$0: debug[3]: CAP_H_FLAG=$CAP_H_FLAG" 1>&2
    echo "$0: debug[3]: CAP_G_FLAG=$CAP_G_FLAG" 1>&2
    echo "$0: debug[3]: CAP_U_FLAG=$CAP_U_FLAG" 1>&2
    echo "$0: debug[3]: D_FLAG=$D_FLAG" 1>&2
    echo "$0: debug[3]: E_FLAG=$E_FLAG" 1>&2
    for index in "${!OPTION[@]}"; do
        echo "$0: debug[$V_FLAG]: OPTION[$index]=${OPTION[$index]}" 1>&2
    done
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
if [[ -z $NOOP ]]; then
    if [[ $V_FLAG -ge 1 ]]; then
	echo "$0: debug[5]: about to execute: $ROGOMATIC_TOOL ${OPTION[*]} --" 1>&2
    fi
    "$ROGOMATIC_TOOL" "${OPTION[@]}" --
    status="$?"
    if [[ $status -ne 0 ]]; then
	if [[ $status -eq 129 ]]; then
	    echo "$0: notice SIGHUP: $ROGOMATIC_TOOL ${OPTION[*]} --" 1>&2
	else
	    echo "$0: Warning: $ROGOMATIC_TOOL ${OPTION[*]} -- failed," \
		 "error code: $status" 1>&2
	    exit 7
	fi
    fi
elif [[ $V_FLAG -ge 3 ]]; then
    echo "$0: debug[3]: because of -n, execution of $ROGOMATIC_TOOL ${OPTION[*]} -- was disabled" 1>&2
fi


# All Done!!! -- Jessica Noll, Age 2
#
exit 0
