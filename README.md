# rogomatic XIV

rogomatic: Automatically Exploring The Dungeons of Doom version XIV


## TL;DR

To make and install:

```sh
make clobber all
sudo make install
```

To run installing:

```sh
./run-rogo
```

An easy way to "gently kill" a running rogomatic session is to run:

```sh
./kill_player
```


## Helper scripts

After you have successfully built everything:

```sh
make clobber all
```

you may wish to use some of the helper scripts:


### rerun-rogo

If one uses the supplied script `./rerun-rogo`, then when Rogomatic (via player) exits, a new Rogomatic will be launched.


### unstuck_player

Due to issue #10, after starting `./rerun-rogo` in one window, also launch `./unstuck_player -v 1` in another window so that when Rogomatic hangs, the session will be "gently terminated" allowing `./rerun-rogo` to start another session.


### kill_player

If all else fails, use `./kill_player` to "gently terminate" the running `rogue(6)` game as well as the `./player` process that was launched by `./rerun-rogo`.


### run-rogo

The `./rerun-rogo` uses the script `./run-rogo` to launch a new Rogomatic controlled `rogue(6)` game.


### Special files and directories


#### .stopfile

This **NOT** a script, but rather a file that will cause `./rerun-rogo`  stop launching new rogomatic controlled `rogue(6)` games.

**HINT**: If you create a file called `.stopfile` ...

```sh
touch .stopfile
```

... while `./rerun-rogo` is rolling, then it will **NOT** start a new session when the current session ends.  This is needed because interrupts (such as via ^C (control C)) are caught by the `rogue(6)` game, or by the `./player` process.  And if you are truly impatient, create the `.stopfile` and then run `./kill_player`.

When `./rerun-rogo` detects the `.stopfile` and ends the rerun loop, it will remove `.stopfile`.


#### /var/tmp/rogo

Tools such as `./run-rogo` (and `./rerun-rogo` which calls `./run-rogo`) use the following **non-default** rogomatic directory (called `RGMDIR`):

```
/var/tmp/rogo
```

The `RGMDIR` is used by `./rogomatic`, `./player`, as well as the `rogue(6)` games is launches.

This means that if you run `rogue(6)` "by hand" using the game's default files (see `man rogue`), you will **NOT** interfere with any  `rogue(6)` game that is being managed by rogomatic.

This also means that the `rogue(6)` score file, and related rogue lock files will be independent.  For example, to print the rogue score file used by ./rerun-rogo` and friends, you need to run:

```sh
rogue -s /var/tmp/rogo/rogue.scr
```


#### rogue

The `rogue(6)` game used by  `./rerun-rogo` and friends is searched for as follows:

1. ./rogue

In the same directory as  `./rerun-rogo` and friends, so you could put a symlink some path to a rogue executable.

2. ../rogue5.4/rogue

If you have cloned `https://github.com/lcn2/rogue5.4` under the same tree as you cloned `https://github.com/lcn2/rogomatic`, and compiled that rogue game, then that execrable will be used.

3. rogue

The `rogue(6)` game will be searched for along your `$PATH` environment variable.

**NOTE**: This rogomatic code is designed to be used with **rogue version 5.4.5 release 2026-06-22**, or any later release of version 5.4.5.  Use of other rogue, especially older rogue games is **NOT** recommended, nor supported.


#### player

The `./rogomatic` binary will also launch the `./player` program, which in turn will play the `rogue(6)` game.

The `./player` command  `./rerun-rogo` and friends is searched for as follows:

1. ./player

In the same directory as  `./rerun-rogo` and friends, so you could put a symlink some path to a rogue executable.

2. player

The `player` command will be searched for along your `$PATH` environment variable.



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


## Why yet another rogomatic repo?

This code is based on the [rogueforge rogomatic14 repo](https://github.com/rogueforge/rogomatic14).

The [rogomatic repo](https://github.com/lcn2/rogomatic) improves on the above mentioned repo in several important aspects:

* Improved the C source to be able to compile under recent C compilers
* Fixed several bugs in the rogue code
* Compatible with the [BSD rogue5.4 repo](https://github.com/lcn2/rogue5.4)
* XXX - TBD - XXX
* etc.



### Bug reports and Pull Requests welcome

We very much welcome fork [rogomatic pull requests](https://github.com/lcn2/rogomatic/pulls) to fix any:

* failure to compile
* compiler warning messages
* program crashes


# Reporting Security Issues

To report a security issue, please visit "[Reporting Security Issues](https://github.com/lcn2/rogomatic/security/policy)".
