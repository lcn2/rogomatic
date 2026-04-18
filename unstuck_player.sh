#!/usr/bin/env bash
#
# unstuck_player.sh - kill rogue if player is stuck
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
export RUNNING_RECHECK_SEC="16"
export RESTART_SEC="4"
export MISSING_RECHECK_SEC="8"
export USR="$USER"


# usage
#
export USAGE="usage: $0 [-h] [-v level] [-V] [-n] [-N] [-r recheck_sec] [-R restart_sec] [-m missing_sec] [-u user]

    -h          print help message and exit
    -v level    set verbosity level (def level: $V_FLAG)
    -V          print version string and exit

    -n		go thru the actions, but do not update any files (def: do the action)
    -N          do not process anything, just parse arguments (def: process something)

    -r recheck_sec	seconds to re-check running player (def: $RUNNING_RECHECK_SEC)
    -R restart_sec	seconds to wait while player restarts (def: $RESTART_SEC)
    -m missing_sec	seconds to re-check for missing player (def: $MISSING_RECHECK_SEC)
    -u user		only process player and rogue run by user (def: run by $USR)

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
while getopts :hv:VnNr:R:m:u: flag; do
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
    r) RUNNING_RECHECK_SEC="$OPTARG"
        ;;
    R) RESTART_SEC="$OPTARG"
        ;;
    m) MISSING_RECHECK_SEC="$OPTARG"
        ;;
    u) USR="$OPTARG"
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
    echo "$0: debug[3]: RUNNING_RECHECK_SEC=$RUNNING_RECHECK_SEC" 1>&2
    echo "$0: debug[3]: RESTART_SEC=$RESTART_SEC" 1>&2
    echo "$0: debug[3]: MISSING_RECHECK_SEC=$MISSING_RECHECK_SEC" 1>&2
fi


# -N stops early before any processing is performed
#
if [[ -n $DO_NOT_PROCESS ]]; then
    if [[ $V_FLAG -ge 3 ]]; then
	echo "$0: debug[3]: arguments parsed, -N given, exiting 0" 1>&2
    fi
    exit 0
fi


# monitor player processes
#
export PS_OUTPUT=
export PREV_PS_OUTPUT=
export PLAYER_WAS_MISSING="true"
while :; do

    # look for the 1st player processes
    #
    # We need the ps output, including time consumed, in order to detect a stalled process
    #
    # SC2009 (info): Consider using pgrep instead of grepping ps output.
    # https://www.shellcheck.net/wiki/SC2009
    # shellcheck disable=SC2009
    PS_OUTPUT=$(ps -U "$USR" -o pid,ppid,time,command | grep -E '[0-9] player [a-z][a-z] [1-9]' | LC_ALL=C sort -n | head -1)

    # case: no player process found, wait a short while
    #
    if [[ -z $PS_OUTPUT ]]; then
	PLAYER_WAS_MISSING="true"
	if [[ $V_FLAG -ge 3 ]]; then
	    echo "$0: debug[3]: no player process found at $(date): sleep $MISSING_RECHECK_SEC" 1>&2
	fi
	sleep "$MISSING_RECHECK_SEC"
	continue
    elif [[ -n $PLAYER_WAS_MISSING ]]; then
	if [[ $V_FLAG -ge 1 ]]; then
	    echo "$0: debug[5]: player now running at $(date): $PS_OUTPUT" 1>&2
	fi
	PLAYER_WAS_MISSING=""
    fi

    # case: player process is accumulating run time
    #
    if [[ $PS_OUTPUT != "$PREV_PS_OUTPUT" ]]; then

	# save current ps output for next cycle
	#
	PREV_PS_OUTPUT="$PS_OUTPUT"

	# report no change if verbose enough
	#
	if [[ $V_FLAG -ge 5 ]]; then
	    echo "$0: debug[5]: player running at $(date): $PS_OUTPUT: sleep $RUNNING_RECHECK_SEC" 1>&2
	fi
	sleep "$RUNNING_RECHECK_SEC"
	continue
    fi

    # case: player process appears to be stuck
    #
    if [[ $V_FLAG -ge 1 ]]; then
	echo "$0: debug[1]: the player stalled at $(date): $PS_OUTPUT" 1>&2
    fi

    # indicate we will HUP rogue
    #
    if [[ $V_FLAG -ge 3 ]]; then
	echo "$0: debug[3]: at $(date): about to: killall -v -q -u $USR -HUP rogue" 1>&2
    fi
    if [[ -z $NOOP ]]; then

	# HUP rogue
	#
	killall -v -q -u "$USR" -HUP rogue 1>&2
	STATUS="$?"

	# case: HUP of rogue failed
	#
	if [[ $STATUS -ne 0 ]]; then

	    # report HUP failed
	    #
	    echo "$0: Warning: killall -v -q -u $USR -HUP rogue failed: error: $STATUS" 1>&2

	    # try to HUP player
	    #
	    if [[ $V_FLAG -ge 1 ]]; then
		echo "$0: debug[1]: at $(date): about to: killall -v -q -u $USR -HUP player" 1>&2
	    fi
	    killall -v -q -u "$USR" -HUP player 1>&2
	    STATUS="$?"
	    if [[ $STATUS -ne 0 ]]; then
		echo "$0: Warning: killall -v -q -u $USR -HUP player failed: error: $STATUS" 1>&2
	    fi

	# case: HUP of rogue successful
	#
	elif [[ $V_FLAG -ge 3 ]]; then
	    echo "$0: debug[3]: player hit with HUP signal at $(date)" 1>&2
	fi

    # -n has disabled use of HUP
    #
    elif [[ $V_FLAG -ge 3 ]]; then
	echo "$0: debug[3]: use of -n disabled hitting player with HUP signal at $(date)" 1>&2
    fi

    # clear previous ps player after rogue kill
    #
    PREV_PS_OUTPUT=""
    PLAYER_WAS_MISSING="true"

    # wait a short period of time to let rogomatic restart both player and rogue
    #
    if [[ $V_FLAG -ge 3 ]]; then
	echo "$0: debug[3]: at $(date): wait for a player restart: sleep $RESTART_SEC" 1>&2
    fi
    sleep "$RESTART_SEC"

done


# All Done!!! -- Jessica Noll, Age 2
#
exit 0
