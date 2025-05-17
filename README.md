# rogomatic


This repository is a fork of Rog-O-Matic, otherwise known as rogomatic.

XXX - This is a port in progress - see the bugs section near the bottom - XXX

Pull requests to help fix are welcome!


## ROG-O-MATIC

See the [ROG-O-MATIC](http://www.cs.princeton.edu/~appel/papers/rogomatic.html) paper from Princeton.


## Origin

Using the [original source](http://www.anthive.com/project/rogue/), a patch was applied from
[http://www.nakajim.net/rogue](http://www.nakajim.net/rogue/).

The above was cloned the [/kngwyu Rog-O-Matic repo](https://github.com/kngwyu/Rog-O-Matic) into
the [lcn2 rogomatic repo](https://github.com/lcn2/rogomatic) and then modified accordingly
in order to port both macOS 15.5 and Red Hat Linux 9.6.


## Dependencies

[Yuji Kanagawa](https://github.com/kngwyu) states that they were able to compile the above
with "GCC 7.2.1 and latest Arch Linux", where the "latest" was probably with respect to
2018 Jan 25 with the original `README.org` was last committed to that repository.

They list the following dependencies:

* gcc
* curses
* gawk
* bison
* flex
* autoconf
* autoreconf

BTW: That original `README.org` was renamed `README.md` and converted into markdown.


## To build

./configure CC="cc --std=c90 -Wno-implicit-function-declaration -Wno-int-conversion -Wno-parentheses -Wno-implicit-int -Wno-return-type -Wno-deprecated-declarations -Wno-format"

make

sudo make install

mkdir -p /var/tmp/rogomatic
chmod 777 /var/tmp/rogomatic
chmod +x ./r-o-m


## To run

./r-o-m 1


## If you must reset to source (do this only if you really need this)

make distclean

env PERL5LIB= PERL_LOCAL_LIB_ROOT= autoreconf -f -i


# Bugs


## General bugs

While running `r-o-m`, instances of `/usr/local/bin/rogue` processes will be orphaned in the background.
To clean up, run:

```sh
killall rogue
```

It is also possible that a number of the compiler warnings, rather than being
silenced via "-Wno-\*" flags, need to be addressed and fixed.


## macOS bugs

The rogomatic tool, via `r-o-m` will crash with a "segmentation fault" after a few seconds on macOS.


## Linux bugs

The rogomatic tool, via `r-o-m` will run somewhat.

At the end of a run, rogomatic will display:

> Unreachable! database/used
