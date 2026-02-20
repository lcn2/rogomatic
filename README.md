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
XXX - TBD - XXX
```


## To configure

If your system as issues compiling this code, edit `Makefile` and/or XXX - TBD - XXX.


# Dependencies

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


## Information that may be of use

XXX - TBD - XXX


### Bug reports and Pull Requests welcome

We very much welcome fork [rogomatic pull requests](https://github.com/lcn2/rogomatic/pulls) to fix any:

* failure to compile
* compiler warning messages
* program crashes


# Reporting Security Issues

To report a security issue, please visit "[Reporting Security Issues](https://github.com/lcn2/rogomatic/security/policy)".
