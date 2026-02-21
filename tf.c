# include <stdio.h>

# include "types.h"
# include "globals.h"

int
main (int argc, char *argv[])
{
  if (argc != 3)
    quit (1, "Usage: tf <rogue-binary> <user-name>\n");

  printf ("Findscore(%s,%s) = %d\n",
	  argv[1], argv[2], findscore (argv[1], argv[2]));
}
