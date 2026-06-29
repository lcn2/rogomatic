#!/usr/bin/env bash
#
# end_player.sh - nicely terminate rogue, and then nicely terminate player
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
export VERSION="1.0.2 2026-06-28"
NAME=$(basename "$0")
export NAME
#
export V_FLAG=0
#
export NOOP=
export DO_NOT_PROCESS=
#
export USR="$USER"


# usage
#
export USAGE="usage: $0 [-h] [-v level] [-V] [-n] [-N] [-u user]

    -h          print help message and exit
    -v level    set verbosity level (def level: $V_FLAG)
    -V          print version string and exit

    -n          go thru the actions, but do not update any files (def: do the action)
    -N          do not process anything, just parse arguments (def: process something)

    -u user     only process player and rogue run by user (def: run by $USR)

Exit codes:
     0         all OK
     2         -h and help string printed or -V and version string printed
     3         command line error
 >= 10         internal error

$NAME version: $VERSION"


# parse command line
#
while getopts :hv:VnNu: flag; do
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
    echo "$0: debug[3]: USER=$USER" 1>&2
    echo "$0: debug[3]: USR=$USR" 1>&2
fi


# -N stops early before any processing is performed
#
if [[ -n $DO_NOT_PROCESS ]]; then
    if [[ $V_FLAG -ge 3 ]]; then
	echo "$0: debug[3]: arguments parsed, -N given, exiting 0" 1>&2
    fi
    exit 0
fi


# attempt to nicely kill rogue, and then nicely kill player
#
# HUP rogue
#
if [[ -z $NOOP ]]; then
    if [[ $V_FLAG -ge 1 ]]; then
	echo "$0: debug[1]: at $(date): about to: killall -v -q -u $USR -HUP rogue" 1>&2
    fi
    killall -v -q -u "$USR" -HUP rogue 1>&2
    STATUS="$?"
else
    if [[ $V_FLAG -ge 1 ]]; then
	echo "$0: debug[1]: use of -n disabled: killall -v -q -u $USR -HUP rogue" 1>&2
    fi
    STATUS=0
fi

# case: HUP of rogue failed
#
if [[ $STATUS -ne 0 ]]; then

    # report HUP failed
    #
    if [[ $STATUS -eq 1 ]]; then
	if [[ $V_FLAG -ge 1 ]]; then
	    echo "$0: debug[1]: no rogue process(es) found" 1>&2
	fi
    else
	echo "$0: Warning: killall -v -q -u $USR -HUP rogue failed: error: $STATUS" 1>&2
    fi

    # sleep a wee bit
    #
    sleep 0.5

    # try to HUP player
    #
    if [[ -z $NOOP ]]; then
	if [[ $V_FLAG -ge 1 ]]; then
	    echo "$0: debug[1]: at $(date): about to: killall -v -q -u $USR -HUP player" 1>&2
	fi
	killall -v -q -u "$USR" -HUP player 1>&2
	STATUS="$?"
    else
	if [[ $V_FLAG -ge 1 ]]; then
	    echo "$0: debug[1]: use of -n disabled: killall -v -q -u $USR -HUP player" 1>&2
	fi
	STATUS=0
    fi
    if [[ $STATUS -ne 0 ]]; then
	if [[ $STATUS -eq 1 ]]; then
	    if [[ $V_FLAG -ge 1 ]]; then
		echo "$0: debug[1]: no player process(es) found" 1>&2
	    fi
	else
	    echo "$0: Warning: killall -v -q -u $USR -HUP player failed: error: $STATUS" 1>&2
	fi
    else
	echo "$0: debug[3]: player hit with HUP signal at $(date)" 1>&2
    fi

# case: HUP of rogue successful
#
elif [[ $V_FLAG -ge 3 && -z $NOOP ]]; then
    echo "$0: debug[3]: rogue hit with HUP signal at $(date)" 1>&2
fi


# All Done!!! -- Jessica Noll, Age 2
#
exit 0
