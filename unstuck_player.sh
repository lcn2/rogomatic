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
export VERSION="1.1.1 2026-07-19"
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
export CPULOOP_SEC="256"
export USR="$USER"
export CAP_A_FLAG=


# usage
#
export USAGE="usage: $0
        [-h] [-v level] [-V] [-n] [-N]
        [-A] [-r recheck_sec] [-R restart_sec] [-u user]

    -h          print help message and exit
    -v level    set verbosity level (def level: $V_FLAG)
    -V          print version string and exit
    -n          go thru the actions, but do not update any files (def: do the action)
    -N          do not process anything, just parse arguments (def: process something)

    -A                  kill player processes using /var/tmp/rogomatic (def: exclude /var/tmp/rogomatic)
    -m missing_sec      seconds to re-check for missing player (def: $MISSING_RECHECK_SEC)
    -r recheck_sec      seconds to re-check running player (def: $RUNNING_RECHECK_SEC)
    -R restart_sec      seconds to wait while player restarts (def: $RESTART_SEC)
    -u user             only process player and rogue run by user (def: run by $USR)
    -z loop_sec         kill player when lvllog not updated for loop_sec (def: $CPULOOP_SEC)

Exit codes:
     0         all OK
     2         -h and help string printed or -V and version string printed
     3         command line error
     5         some internal tool is not found or not an executable file
     6         problems found with or in the rogomatic directory
 >= 10         internal error

$NAME version: $VERSION"


# parse command line
#
while getopts :hv:VnNAr:R:m:u: flag; do
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

    A) CAP_A_FLAG="-n"
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
    echo "$0: debug[3]: CPULOOP_SEC=$CPULOOP_SEC" 1>&2
    echo "$0: debug[3]: USER=$USER" 1>&2
    echo "$0: debug[3]: USR=$USR" 1>&2
    echo "$0: debug[3]: CAP_A_FLAG==$CAP_A_FLAG=" 1>&2
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
export ROGUE_PID=
export RGMDIR_USED=
export ROGUE_WAS_KILLED=
while :; do

    # look for the 1st player processes
    #
    # We need the ps output, including time consumed, in order to detect a stalled process
    #
    # We exclude any player running with the original /var/tmp/rogomatic path, unless -A is used.
    #
    if [[ -n $CAP_A_FLAG ]]; then
	# We need a special scan that pgrep(1) doesn't have,
	# at least the portable version of pgrep(1) doesn't.
	# Same thing for the pidof command.
	#
	# SC2009 (info): Consider using pgrep instead of grepping ps output.
	# https://www.shellcheck.net/wiki/SC2009
	# shellcheck disable=SC2009
	PS_OUTPUT=$(ps -U "$USR" -o pid,ppid,time,command |
	            grep -E '[0-9] player [a-z][a-z] [1-9]' |
		    grep -E -v ' /var/tmp/rogomatic$' |
		    LC_ALL=C sort -n |
		    head -1)
    else
	# We need a special scan that pgrep(1) doesn't have,
	# at least the portable version of pgrep(1) doesn't.
	# Same thing for the pidof command.
	#
	# SC2009 (info): Consider using pgrep instead of grepping ps output.
	# https://www.shellcheck.net/wiki/SC2009
	# shellcheck disable=SC2009
	PS_OUTPUT=$(ps -U "$USR" -o pid,ppid,time,command |
	            grep -E '[0-9] player [a-z][a-z] [1-9]' |
		    LC_ALL=C sort -n |
		    head -1)
    fi

    # case: no player process found, wait a short while
    #
    if [[ -z $PS_OUTPUT ]]; then
	PLAYER_WAS_MISSING="true"
	if [[ $V_FLAG -ge 3 ]]; then
	    echo "$0: debug[3]: no player process found at $(date): sleep $MISSING_RECHECK_SEC" 1>&2
	fi
	sleep "$MISSING_RECHECK_SEC"
	continue
    elif [[ -n $PLAYER_WAS_MISSING || -n $ROGUE_WAS_KILLED ]]; then
	if [[ $V_FLAG -ge 1 ]]; then
	    echo "$0: debug[1]: player now running at $(date): $PS_OUTPUT" 1>&2
	fi
	PLAYER_WAS_MISSING=""
	ROGUE_WAS_KILLED=""
    fi

    # extract the rogue process ID for this player
    #
    # NOTE: The 3rd arg of the player process,
    # and thus the 6th of the ps player output is the pid of the rogue process.
    #
    ROGUE_PID=$(echo "$PS_OUTPUT" | awk '{print $6;}')

    # extract the player process ID
    #
    # The 1st arg of the ps player output is the player process id.
    #
    PLAYER_PID=$(echo "$PS_OUTPUT" | awk '{print $1;}')

    # extract the rogomatic directory
    #
    # The last arg of the ps player output is the rogomatic directory being used.
    #
    RGMDIR_USED=$(echo "$PS_OUTPUT" | awk '{print $NF;}')

    # case: player process is accumulating run time
    #
    if [[ $PS_OUTPUT != "$PREV_PS_OUTPUT" ]]; then

	# save current ps output for next cycle
	#
	PREV_PS_OUTPUT="$PS_OUTPUT"

	# report no change if verbose enough
	#
	if [[ $V_FLAG -ge 5 ]]; then
	    echo "$0: debug[5]: player running at $(date): $PS_OUTPUT" 1>&2
	fi

	# detect if the rogue process is stuck in a CPU-bound loop
	#
	if [[ -z $RGMDIR_USED ]]; then
	    echo "$0: warning: RGMDIR_USED is empty" 1>&2
	    continue
	fi
	if [[ ! -d $RGMDIR_USED ]]; then
	    echo "$0: warning: RGMDIR_USED: not a directory: $RGMDIR_USED" 1>&2
	    continue
	fi
	TOTAL_LVLLOG="$RGMDIR_USED/total.lvllog"
	export TOTAL_LVLLOG
	if [[ ! -f $TOTAL_LVLLOG ]]; then
	    echo "$0: warning: TOTAL_LVLLOG: not a file: $TOTAL_LVLLOG" 1>&2
	    continue
	fi
	MODTIME=$(stat -c '%Y' "$TOTAL_LVLLOG" 2>/dev/null)
	export MODTIME
	if [[ -z $MODTIME ]]; then
	    echo "$0: warning: MODTIME: is empty" 1>&2
	    continue
	fi
	if [[ ! $MODTIME =~ ^[0-9]+$ ]]; then
	    echo "$0: warning: MODTIME: is not an integer: $MODTIME" 1>&2
	    continue
	fi
	NOW=$(date '+%s' 2>/dev/null)
	export NOW
	if [[ -z $NOW ]]; then
	    echo "$0: warning: NOW is empty" 1>&2
	    continue
	fi
	if [[ ! $NOW =~ ^[0-9]+$ ]]; then
	    echo "$0: warning: NOW: is not an integer: $NOW" 1>&2
	    continue
	fi
	((MOD_SECS=NOW-MODTIME))
	export MOD_SECS
	if [[ -z $MOD_SECS ]]; then
	    echo "$0: warning: MOD_SECS is empty" 1>&2
	    continue
	fi
	if [[ ! $MOD_SECS =~ ^[0-9]+$ ]]; then
	    echo "$0: warning: MOD_SECS: is not an integer: $NOW" 1>&2
	    continue
	fi

	# unless we seem to be stuck on the level
	#
	if [[ $MOD_SECS -lt $CPULOOP_SEC ]]; then

	    # wait a bit before checking again
	    #
	    if [[ $V_FLAG -ge 5 ]]; then
		echo "$0: debug[5]: sleep $RUNNING_RECHECK_SEC" 1>&2
	    fi
	    sleep "$RUNNING_RECHECK_SEC"
	    continue
	fi
    fi

    # case: player process appears to be stuck
    #
    if [[ $V_FLAG -ge 1 ]]; then
	if [[ $MOD_SECS -lt $CPULOOP_SEC ]]; then
	    echo "$0: debug[1]: player stalled at $(date): $PS_OUTPUT" 1>&2
	else
	    echo "$0: debug[1]: player looping at $(date): $PS_OUTPUT" 1>&2
	fi
    fi

    # attempt to HUP rogue
    #
    if [[ -n $ROGUE_PID ]]; then

	# pre-check if the rogue pid is still running
	#
	# NOTE: If is OK if the rogue pid exits between the check and
	#	the when we try to SIGHUP.
	#
	if kill -0 "$ROGUE_PID" 2>/dev/null; then
	    if [[ -z $NOOP ]]; then

		# HUP rogue
		#
		ROGUE_WAS_KILLED="true"
		echo "$0: notice: for rogue: kill -HUP $ROGUE_PID" 1>&2
		kill -HUP "$ROGUE_PID" 2>/dev/null
		STATUS="$?"

		# case: HUP of rogue failed
		#
		# NOTE: Because player may exit before try to SIGHUP it, we ignore kill errors
		#
		if [[ $STATUS -eq 0 ]]; then
		    if [[ $V_FLAG -ge 3 ]]; then
			echo "$0: debug[3]: rogue hit with HUP signal at $(date)" 1>&2
		    fi

		    # sleep a wee bit to let the process exit
		    #
		    sleep 0.5
		fi

	    # -n has disabled use of HUP
	    #
	    elif [[ $V_FLAG -ge 3 ]]; then
		echo "$0: debug[3]: use of -n disabled hitting rogue pid $ROGUE_PID with HUP signal at $(date)" 1>&2
	    fi
	else
	    echo "$0: notice: rogue already exited: pid: $ROGUE_PID" 1>&2
	fi
    fi

    # attempt to HUP player
    #
    if [[ -n $PLAYER_PID ]]; then

	# pre-check if the player pid is still running
	#
	# NOTE: If is OK if the player pid exits between the check and
	#	the when we try to SIGHUP.
	#
	if kill -0 "$PLAYER_PID" 2>/dev/null; then
	    if [[ -z $NOOP ]]; then

		# HUP player
		#
		ROGUE_WAS_KILLED="true"
		echo "$0: notice: for player: kill -HUP $PLAYER_PID" 1>&2
		kill -HUP "$PLAYER_PID" 2>/dev/null
		STATUS="$?"

		# report player HUP success
		#
		# NOTE: Because player may exit before try to SIGHUP it, we ignore kill errors
		#
		if [[ $STATUS -eq 0 ]]; then
		    if [[ $V_FLAG -ge 3 ]]; then
			echo "$0: debug[3]: player hit with HUP signal at $(date)" 1>&2
		    fi

		    # sleep a wee bit to let the process exit
		    #
		    sleep 0.5
		fi

	    # -n has disabled use of HUP
	    #
	    elif [[ $V_FLAG -ge 3 ]]; then
		echo "$0: debug[3]: use of -n disabled hitting player pid $PLAYER_PID with HUP signal at $(date)" 1>&2
	    fi
	else
	    echo "$0: notice: player already exited: pid: $PLAYER_PID" 1>&2
	fi
    fi

    # deal with a stubborn rogue process that failed to exit, unless -n was used
    #
    if [[ -z $NOOP ]]; then
	if kill -0 "$ROGUE_PID" 2>/dev/null; then

	    # KILL rogue
	    #
	    ROGUE_WAS_KILLED="true"
	    echo "$0: notice: rogue still running, will: kill -KILL $ROGUE_PID" 1>&2
	    kill -KILL "$ROGUE_PID" 2>/dev/null
	    STATUS="$?"

	    # report rogue KILL success
	    #
	    # NOTE: Because rogue may exit before try to SIGKILL it, we ignore kill errors
	    #
	    if [[ $STATUS -eq 0 ]]; then
		if [[ $V_FLAG -ge 3 ]]; then
		    echo "$0: debug[3]: rogue hit with KILL signal at $(date)" 1>&2
		fi
	    fi
	fi
    fi

    # deal with a stubborn player process that failed to exit, unless -n was used
    #
    if [[ -z $NOOP ]]; then
	if kill -0 "$PLAYER_PID" 2>/dev/null; then

	    # KILL player
	    #
	    ROGUE_WAS_KILLED="true"
	    echo "$0: notice: player still running, will: kill -KILL $PLAYER_PID" 1>&2
	    kill -KILL "$PLAYER_PID" 2>/dev/null
	    STATUS="$?"

	    # report player KILL success
	    #
	    # NOTE: Because player may exit before try to SIGKILL it, we ignore kill errors
	    #
	    if [[ $STATUS -eq 0 ]]; then
		if [[ $V_FLAG -ge 3 ]]; then
		    echo "$0: debug[3]: player hit with KILL signal at $(date)" 1>&2
		fi
	    fi

	# stubborn process (rogue and player) have been handled, return to processing
	#
	else
	    continue
	fi
    fi

    # clear ps state after rogue and/or player are HUP-ed
    #
    PREV_PS_OUTPUT=""

    # wait a short period of time to let rerun_rogo restart both player and rogue
    #
    if [[ $V_FLAG -ge 3 ]]; then
	echo "$0: debug[3]: at $(date): wait for a player restart: sleep $RESTART_SEC" 1>&2
    fi
    sleep "$RESTART_SEC"

done


# All Done!!! -- Jessica Noll, Age 2
#
exit 0
