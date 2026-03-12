/*
 * install.h: Rog-O-Matic XIV (CMU) Thu Jul  3 15:32:49 1986 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * This file contains (hopefully) all system dependent defines
 * This version of Rog-O-Matic runs with Rogue version 5.3.
 */


#if !defined(INCLUDE_INSTALL_H)
#define INCLUDE_INSTALL_H


/*
 * Rog-O-Matic will not try to beat scores higher than BOGUS which
 * appear on the Rogue scoreboard.
 */

# define BOGUS		(999999)

/*
 * This variable defines the version of Rogue we are assumed to be playing
 * if the getrogueversion() routine can't figure it out.  This must be
 * defined, and can be either "5.2", "3.6", or "5.3". DEFRV is must be
 * the corresponding internal version code defined in types.h.
 */

# define DEFVER		"5.4.5"
# define DEFRV		RV54B

/*
 * This file is created whenever the Rog-O-Matic score file is accessed to
 * prevent simultaneous accesses. This variable must be defined, but will
 *rogomatic.cat.in not be used unless RGMDIR is also defined.
 */

# if defined(RGMDIR)
#   define LOCKFILE	RGMDIR "/Rgm.Lock"
# else
#   define LOCKFILE	"/tmp/Rgm.Lock"
# endif

/*
 * This variable is the level at which we always start logging the game
 */

# define GOODGAME	(18)

/*
 * This variable defines the "local" copy of Rogue, which may have only
 * intermittent access.  This was useful at CMU, since Rogue 5.3 was not
 * supported on all machines.  First Rog-O-Matic looks for "rogue" in the
 * current directory, then this file is used.  This variable need not be
 * defined.
 */

# undef NEWROGUE

/*
 * This is the location of the player executable, which is the main
 * process for Rog-O-Matic.  If "player" does not exist in the current
 * directory, then this file is used. This variable need not be defined
 * (but in that case there must be a "player" binary in the current
 * directory).
 */

# define PLAYER		"/usr/local/bin/player"

/*
 * This is the version of the "current" Rog-O-Matic, and is an uppercase
 * Roman numeral.  It must be defined.
 */

# define RGMVER		"XIV"

/*
 * This is the standard system version of Rogue, and is used if "rogue"
 * and NEWROGUE are not available. It need not be defined, but if it is
 * not, and NEWROGUE is not defined, then there must be a "rogue" in the
 * current directory.
 */

# if !defined(ROGUE)
#   define ROGUE	"/usr/local/bin/rogue"
# endif

/*
 * This file is created in the RGMDIR or current directory if the logging option is
 * enabled.  If the game terminates normally, this file is renamed to
 * <killer>.<level>.<score>.  This variable must be defined.
 */

# if defined(RGMDIR)
# define ROGUELOG	RGMDIR "/roguelog"
# else
# define ROGUELOG	"./roguelog"
# endif

/*
 * This directory must be defined.  It will contain logs of Rogomatic's
 * scores, an error.log file, and the long term memory file.  It must
 * be writable by everyone, since score files must be created and
 * destroyed by anyone running the program.  Alternatively, the
 * player process could be made setuid, with that uid owning this
 * directory.
 */

# if !defined(RGMDIR)
#   define RGMDIR	"/usr/local/tmp/rogomatic"
# endif

/*
 * This file is created in the current directory if the snapshot command
 * is typed during a Rogue game.  It must be defined.
 */

# define SNAPSHOT	"./snapshot.rgm"


#endif
