/*
 * timer - timeout timer related functions
 *
 * Copyright (c) 2026 by Landon Curt Noll.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright, this permission notice and text
 * this comment, and the disclaimer below appear in all of the following:
 *
 *       supporting documentation
 *       source copies
 *       source works derived from this source
 *       binaries derived from this source or from derived source
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * chongo (Landon Curt Noll) /\oo/\
 *
 * http://www.isthe.com/chongo/index.html
 * https://github.com/lcn2
 *
 * Share and enjoy!  :-)
 */

# include <stdio.h>
# include <signal.h>
# include <string.h>

# define HAVE_TIMER_INCLUDES
# include <sys/time.h>	/* for struct itimerval */
# include <setjmp.h>	/* for sigjmp_buf */
# include <errno.h>

# include "types.h"
# include "config.h"
# include "globals.h"


/*
 * global declarations
 */

/* Single shot timeout timer and timer jump point */
struct itimerval timer = {      /* single shot timeout timer in seconds if > 0.0 seconds */
    .it_interval = {0, 0},
    .it_value = {0, 0}
};
bool jmp_point_ready = false;	/* True if non-local jump point for SIGARLM signals has been setup */
sigjmp_buf jmp_point;		/* non-local jump point for SIGARLM signals if jmp_point_ready is true */


/*
 * static declarations
 */

static void alarm_handler (int sig __attribute__ ((__unused__)));


/*
 * is_timer_active - check if the timeout timer is > 0.0 seconds
 */
bool
is_timer_active (void)
{
    bool ret;	    /* return value */

    ret = (timer.it_value.tv_sec > 0 || timer.it_value.tv_usec > 0);
    return ret;
}

/*
 * alarm_handler - signal handler for SIGALRM
 *
 * Jump back to non-local jmp_point if jmp_point_ready is true.
 */
static void
alarm_handler (int sig __attribute__ ((__unused__)))
{
  /*
   * non-local jump back if the jmp_point is ready
   */
  if (jmp_point_ready) {
    /* jump back to the saved point in the processing loop, restoring signal mask */
    siglongjmp (jmp_point, 1);
  }

  /* otherwise do nothing about the SIGALRM as the jmp_point is NOT ready */
  return;
}

/*
 * enable_alarm_use - set up the special SIGALRM signal handler if timeout timer > 0.0 seconds
 */
void
enable_alarm_use (void)
{
  struct sigaction sa;	/* SIGALRM action */

  /*
   * do nothing unless timeout timer is > 0.0 seconds
   */
  if (! is_timer_active ()) {
    return;
  }

  /*
   * initialize SIGALRM action
   */
  memset (&sa, 0, sizeof(sa));
  sa.sa_handler = alarm_handler;
  /* avoid SA_RESTART so blocking calls like read calls can be interrupted if needed */
  sa.sa_flags = 0;
  sigemptyset (&sa.sa_mask);

  /*
   * setup special SIGALRM signal action
   */
  if (sigaction (SIGALRM, &sa, NULL) == -1) {
    quit (1, "ERROR: %s: file: %s line: %d dungeon: %u failed set sigaction for SIGALRM: %s\n",
	     __func__, __FILE__, __LINE__, dnum, strerror (errno));
    not_reached ();
  }

  return;
}

/*
 * disable_alarm_use - ignore the special SIGALRM signal
 */
void
disable_alarm_use (void)
{
  struct sigaction sa;	/* SIGALRM action */

  /*
   * initialize SIGALRM action
   */
  memset (&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  sigemptyset (&sa.sa_mask);

  /* use SA_RESTART so blocking calls like read will be resumed */
  sa.sa_flags = SA_RESTART;

  /*
   * special SIGALRM signal action is disabled
   */
  if (sigaction (SIGALRM, &sa, NULL) == -1) {
    quit (1, "ERROR: %s: file: %s line: %d dungeon: %u failed disable sigaction for SIGALRM: %s\n",
	     __func__, __FILE__, __LINE__, dnum, strerror (errno));
    not_reached ();
  }

  return;
}

/*
 * set_alarm - set single-shot SIGALRM alarm if timeout timer > 0.0 seconds, else clear SIGALRM alarm
 *
 * NOTE: We don't set an timeout timer if the jmp_point is NOT ready.
 */
void
set_alarm (void)
{
  int ret;              /* setitimer return */

  /*
   * nothing do if the jmp_point is NOT ready
   */
  if (! jmp_point_ready) {
    return;
  }

  /*
   * set the single-shot SIGALRM alarm if timeout timer > 0.0 seconds
   */
  if (is_timer_active ()) {
    ret = setitimer (ITIMER_REAL, &timer, NULL);
    if (ret < 0) {
      quit (1, "ERROR: %s: file: %s line: %d dungeon: %u failed to set alarm: %s\n",
	       __func__, __FILE__, __LINE__, dnum, strerror (errno));
      not_reached ();
    }

  /*
   * clear the single-shot alarm if timer <= 0.0 seconds
   */
  } else {
    clear_alarm ();
  }

  return;
}

/*
 * clear_alarm - clear/cancel any single-shot SIGALRM alarm
 */
void
clear_alarm (void)
{
  struct itimerval disable_timer = {	    /* zero SIGARLM timer */
      .it_interval = {0, 0},
      .it_value = {0, 0}
  };
  int ret;				    /* setitimer return */

  /* clear the single-shot SIGALRM alarm */
  ret = setitimer (ITIMER_REAL, &disable_timer, NULL);
  if (ret < 0) {
    quit (1, "ERROR: %s: file: %s line: %d dungeon: %u failed disable alarm: %s\n",
	     __func__, __FILE__, __LINE__, dnum, strerror (errno));
    not_reached ();
  }

  return;
}
