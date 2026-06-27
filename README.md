# TL;DR

To make and install:

```sh
make clobber all
sudo make install
```

To run one instance of rogomatic, try:

```sh
./run-rogo
```

An easy way to "gently kill" a running rogomatic session is to run:

```sh
./end_player
```

To have rogomatic continuously start another rogue game after the explorer dies:

```sh
./rerun-rogo
```

It is recommended that while `./rerun-rogo` is running, in another window run:

```sh
./unstuck_player -v 1
```

Also creating the local file, `.stopfile`:

```sh
touch .stopfile
```

will cause `./rerun-rogo` to stop restarting rogomatic when `rogue(6)` exits.


## IMPORTANT: Use with rogue 5.4.5

This rogomatic code is designed to be used with **rogue version
5.4.5 release 2026-06-22**, or any later release of version 5.4.5.

See [rogue version 5.4.5](https://github.com/lcn2/rogue5.4) for the
latest version of rogue that is supported by rogomatic.

Use of other rogue, especially older rogue games is **NOT** recommended, nor supported.


# Helper scripts

After you have successfully built everything:

```sh
make clobber all
```

you may wish to use some of the helper scripts:


## rerun-rogo

If one uses the supplied script `./rerun-rogo`, then when the `rogue(6)` game ends
(say because the rogue dies in the dungeon), then a new Rogomatic will be launched.

To use:

```sh
./rerun-rogo
```

It is recommended that while `./rerun-rogo` is running, in another window run:

```sh
./unstuck_player -v 1
```


## unstuck_player

Due to the [issue #10](https://github.com/lcn2/rogomatic/issues/10) bug,
after starting `./rerun-rogo` in one window, also launch `./unstuck_player`
in another window so that when Rogomatic hangs, the session will be
"gently terminated" allowing `./rerun-rogo` to start another session.

While `./rerun-rogo` is running, in another window try:

```sh
./unstuck_player -v 1
```

This has the added advantage of allowing `./player` to update (if needed),
the genetic state, and to allow the `rogue(6)` game to update (if needed),
tor score file.  In some cases, the rogue save file may be used to
manually restart the rogue game.


## end_player

You may find it challenging to terminate the running `rogue(6)` game as
well as the `./player` process that was launched by `./rerun-rogo`.

Use `./end_player` to "gently terminate" the running `rogue(6)` game
as well as the `./player` process that was launched by `./rerun-rogo`.
This has the added advantage of allowing `./player` to update (if needed),
the genetic state, and to allow the `rogue(6)` game to update (if needed),
tor score file.  In some cases, the rogue save file may be used to
manually restart the rogue game.


## run-rogo

The `./rerun-rogo` uses the script `./run-rogo` to launch a new Rogomatic
controlled `rogue(6)` game.


## Special files and directories


### .stopfile

This **NOT** a script, but rather a file that will cause `./rerun-rogo`
to
to stop launching new Rogomatic controlled `rogue(6)` games.

**HINT**: If you create a file called `.stopfile` ...

```sh
touch .stopfile
```

... while `./rerun-rogo` is rolling, then it will **NOT** start a new
session when the current session ends.  This is needed because interrupts
(such as via ^C (control C)) are caught by the `rogue(6)` game, or by
the `./player` process.  And if you are truly impatient, create the
`.stopfile` and then run `./end_player`.

When `./rerun-rogo` detects the `.stopfile` and ends the rerun loop,
it will remove `.stopfile`.


### /var/tmp/rogo

Tools such as `./run-rogo` (and `./rerun-rogo` which calls `./run-rogo`)
use the following **non-default** rogomatic directory (called `RGMDIR`):

```
/var/tmp/rogo
```

The `RGMDIR` is used by `./rogomatic`, `./player`, as well as the
`rogue(6)` games that are launched.

This means that if you run `rogue(6)` "by hand" using the game's default
files (see `man rogue`), you will **NOT** interfere with any `rogue(6)`
game that is being managed by rogomatic.

This also means that the `rogue(6)` score file, and related rogue lock
files will be independent.  For example, to print the rogue score file
used by ` ./rerun-rogo` and friends, you need to run:

```sh
rogue -s /var/tmp/rogo/rogue.scr
```


### rogue

The `rogue(6)` game used by `./rerun-rogo` and friends is searched for as follows:

1. ./rogue

In the same directory as `./rerun-rogo` and friends, so you could put
a symlink to some path to a rogue executable.

2. ../rogue5.4/rogue

If you have cloned
[https://github.com/lcn2/rogue5.4](https://github.com/lcn2/rogue5.4)
under the same parent directory that you cloned
[https://github.com/lcn2/rogomatic](https://github.com/lcn2/rogomatic),
and compiled that rogue game, then that executable will be used.

3. rogue

The `rogue(6)` game will be searched for along your `$PATH` environment variable.


### player

The `./rogomatic` binary will also launch the `./player` program, which
in turn will play the `rogue(6)` game.

The `./player` command used by  `./rerun-rogo` and friends is searched for as follows:

1. ./player

In the same directory as `./rerun-rogo` and friends, so you could put
a symlink to some path to a rogue executable.

2. player

The `player` command will be searched for along your `$PATH` environment variable.


## For more information

See the `rogomatic(6)` man page for additional information on how to run rogomatic.

After compiling, and prior to installing, you may review the local copy of the man page:

```sh
man ./rogomatic.6
```


## To configure

If your system as issues compiling this code, try editing `Makefile` as needed.


## Dependencies

* Modern C compiler (c17/gnu17 or better)
* Modern `make(1)` (recommend [GNU make](https://www.gnu.org/software/make/))
* [Ncurses](https://invisible-island.net/ncurses/announce.html) (<ncurses.h> and libncurses), or for NetBSD,  NetBSD curses is likely to work
* [Single UNIX Specification](https://pubs.opengroup.org/onlinepubs/9799919799/) confirming (or reasonably conforming) operating system such as Linux, macOS, BSD, etc.


## Compatibility and Source Code Origin

The [rogomatic repo](https://github.com/lcn2/rogomatic) started out as a clone of the
[rogueforge rogomatic14 repo](https://github.com/rogueforge/rogomatic14).

**IMPORTANT NOTE**: This rogomatic code is designed to be used with **rogue version
5.4.5 release 2026-06-22**, or any later release of version 5.4.5.

See [rogue version 5.4.5](https://github.com/lcn2/rogue5.4) for the
latest version of rogue that is supported by rogomatic.

Use of other rogue, especially older rogue games is **NOT** recommended, nor supported.


## Why yet another rogomatic repo?

This code is based on the [rogueforge rogomatic14 repo](https://github.com/rogueforge/rogomatic14).

The [rogomatic repo](https://github.com/lcn2/rogomatic) improves on the above mentioned repo in several important aspects:

* Improved the C source to be able to compile under recent C compilers
* Fixed several bugs in the rogue code
* Compatible with the [BSD rogue5.4 repo](https://github.com/lcn2/rogue5.4)
* etc.


### Bug reports and Pull Requests welcome

We very much welcome fork [rogomatic pull requests](https://github.com/lcn2/rogomatic/pulls) to fix any:

* failure to compile
* compiler warning messages
* program crashes


# Reporting Security Issues

To report a security issue, please visit "[Reporting Security Issues](https://github.com/lcn2/rogomatic/security/policy)".
