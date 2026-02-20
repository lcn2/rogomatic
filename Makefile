#!/usr/bin/env make
#
# rogomatic - Rog-O-Matic XIV
#
# The following Copyright ONLY applies to this Makefile and README.md files.
#
# Copyright (c) 2025-2026 by Landon Curt Noll.  All Rights Reserved.
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


#############
# utilities #
#############

CC= cc
CHGRP= chgrp
CHMOD= chmod
CMP= cmp
COLCRT= colcrt
CP= cp
CTAGS= ctags
GREP= grep
GROFF= groff
ID= id
INSTALL= install
LEX= lex
MAKEDEPEND= makedepend
MKDIR= mkdir
MV= mv
NROFF= nroff
RM= rm
RMDIR= rmdir
SED= sed
SHELL= bash
SORT= sort
TBL= tbl
TOUCH= touch
TRUE= true


#################
# configuration #
#################

PREFIX= /usr/local
BINDIR= ${PREFIX}/bin
DESTDOC= ${PREFIX}/share/rogomatic
MAN6DIR= ${PREFIX}/man/man6


#####################
# debug information #
#####################

# V=@:  do not echo debug statements (quiet mode)
# V=@   echo debug statements (debug / verbose mode)
#
V=@:
#V=@

# Q=@   do not echo internal Makefile actions (quiet mode)
# Q=    echo internal Makefile actions (debug / verbose mode)
#
#Q=
Q=@

# S= >/dev/null 2>&1    silence ${CC} output during hsrc file formation
# S=                    full ${CC} output during hsrc file formation
#
S= >/dev/null 2>&1
#S=


############################
# how to build and install #
############################

CSTD= -std=gnu17
CCWARN=
CPPFLAGS=
COPT= -O3
DEBUG= -ggdb3

CFLAGS= ${CSTD} ${CCWARN} ${CPPFLAGS} ${COPT} ${DEBUG}

LDFLAGS= ${DEBUG}

LC_CTYPE= C

AT= @

# The NetBSD maintainers insist in using curses instead of ncurses.
# NetBSD uses their own curses: a hybrid of BSD curses and X/Open curses.
# There are known incompatibles between NetBSD curses and what the next of the
# world calls ncurses. See https://wiki.netbsd.org/curses_in_netbsd/
# where not only does the NetBSD maintainers admit to being one of the
# last operating systems (or even the last one?) not using ncurses, and
# where they indicate what is different with ncurses.  One should also
# read: https://www.invisible-island.net/ncurses/ncurses-netbsd.html
# for more info.  Well if you use NetBSD, we hope the NetBSD curses
# works for you.
#
# Alternatively NetBSD users can install the ncurses package for
# NetBSD: https://cdn.netbsd.org/pub/pkgsrc/current/pkgsrc/devel/ncurses/README.html
#
target=$(shell uname -s 2>/dev/null)
ifeq ($(target),NetBSD)
LIBS= -lcurses
else
LIBS= -lncurses
endif

OTHER_TARGES= anim testfind genetest gplot tf

TARGETS= rogomatic player rgmplot datesub histplot gene rogomatic.6 rogomatic.cat


################################
# source and destination files #
################################

H_SRC= types.h globals.h install.h termtokens.h

OBJS= arms.o command.o database.o debug.o explore.o io.o learn.o \
	ltm.o main.o mess.o monsters.o pack.o rand.o replay.o rooms.o \
	scorefile.o search.o stats.o strategy.o survival.o tactics.o \
	things.o titlepage.o utility.o worth.o

CFILES= arms.c command.c database.c debug.c explore.c io.c learn.c \
	ltm.c main.c mess.c monsters.c pack.c rand.c replay.c rooms.c \
	scorefile.c search.c stats.c strategy.c survival.c tactics.c \
	things.c titlepage.c utility.c worth.c

MISC_C= setup.c findscore.c histplot.c rgmplot.c gene.c tf.c

C_SRC= ${CFILES} ${MISC_C}

SRC= ${C_SRC} ${H_SRC}

MISC_OBJS= datesub.o gene.o histplot.o rgmplot.o setup.o findscore.o anim.o \
	   testfind.o genetest.o gplot.o tf.o

OTHERS= datesub.l rplot rgmhist rgmscatter

MISC_DOC= knowledge LICENSE ORIG_CHANGES ORIG_CHANGES ORIG_COPYRGHT ORIG_MANPAGE \
	ORIG_README README.md rnotes rogomatic.6.in rogomatic.cat.in rogo.mss \
	rogtitle.mss rterm.tic strike.mss title.ani titlepage.ani title.start \
	CODE_OF_CONDUCT.md CONTRIBUTING.md SECURITY.md

MAKE_FILE= Makefile


###################################
# Common Address Sanitizer (ASAN) #
###################################

# Common Address Sanitizer (ASAN)
#
# For more info see: https://github.com/google/sanitizers/wiki/AddressSanitizer
# See also: https://developer.apple.com/documentation/xcode/diagnosing-memory-thread-and-crash-issues-early
# And also: https://github.com/google/sanitizers/wiki/AddressSanitizerFlags
#
# The following Address Sanitizer (ASAN) are common to both RHEL9.7 (Linux) and macOS 26.2.
#
# By default, the Address Sanitizer is NOT enabled.
#
FSANITIZE:= -Wno-invalid-command-line-argument
FSANITIZE+= -fno-common
FSANITIZE+= -fno-omit-frame-pointer
FSANITIZE+= -fsanitize=address
FSANITIZE+= -fsanitize=alignment
FSANITIZE+= -fsanitize=bool
FSANITIZE+= -fsanitize=bounds
FSANITIZE+= -fsanitize=enum
FSANITIZE+= -fsanitize=float-cast-overflow
FSANITIZE+= -fsanitize=float-divide-by-zero
FSANITIZE+= -fsanitize=integer-divide-by-zero
FSANITIZE+= -fsanitize=nonnull-attribute
FSANITIZE+= -fsanitize=null
FSANITIZE+= -fsanitize=object-size
FSANITIZE+= -fsanitize=returns-nonnull-attribute
FSANITIZE+= -fsanitize=shift
FSANITIZE+= -fsanitize=signed-integer-overflow
FSANITIZE+= -fsanitize=undefined
FSANITIZE+= -fsanitize=unreachable
FSANITIZE+= -fsanitize=vla-bound
FSANITIZE+= -fsanitize=vptr

####
# macOS Address Sanitizer (ASAN)
#
# This comment block was tested under:
#
#       macOS 26.2 with Apple clang version 17.0.0 (clang-1700.6.3.2)
#
#       See: https://releases.llvm.org/17.0.1/tools/clang/docs/AddressSanitizer.html
#
# To use the Address Sanitizer, uncomment this set set of lines and recompile (make clobber all):
#
# For more info see: https://github.com/google/sanitizers/wiki/AddressSanitizer
# See also: https://developer.apple.com/documentation/xcode/diagnosing-memory-thread-and-crash-issues-early
# And also: https://github.com/google/sanitizers/wiki/AddressSanitizerFlags
#
# For Homebrew gcc version 15 only:
#
# CC:= gcc-15
# DEBUG:= -g2
#
# For Apple clang and Homebrew gcc:
#
# WARN+= -Wall
# CCWARN+= -pedantic
# CCWARN+= -Werror
# COPT:= -O0
#
# For Apple clang only:
#
# FSANITIZE+= -fsanitize=nullability-arg
# FSANITIZE+= -fsanitize=nullability-assign
# FSANITIZE+= -fsanitize=nullability-return
#
# CFLAGS+= ${FSANITIZE} -fstack-protector-all
# LDFLAGS+= ${FSANITIZE}
# DEBUG:= -ggdb3
####

####
# RHEL (Linux) Address Sanitizer (ASAN)
#
# This comment block was tested under:
#
#       RHEL9.7 with clang version 20.1.8 (Red Hat, Inc. 20.1.8-3.el9)
#
# with these RPMs installed:
#
#       libasan-11.5.0-11.el9.x86_64
#       libubsan-11.5.0-11.el9.x86_64
#
# To use the Address Sanitizer, uncomment this set set of lines and recompile (make clobber all):
#
# For more info see: https://github.com/google/sanitizers/wiki/AddressSanitizer
# And also: https://github.com/google/sanitizers/wiki/AddressSanitizerFlags
#
# Be sure you have the following dnf packages installed:
#
#   dnf install readline-devel ncurses-libs ncurses-devel libasan libubsan
#
# CCWARN+= -Wall
# CCWARN+= -pedantic
# CCWARN+= -Werror
# COPT:= -O0
#
# CFLAGS+= ${FSANITIZE} -fstack-protector-all
# LDFLAGS+= ${FSANITIZE}
# DEBUG:= -ggdb3
####


######################################
# all - default rule - must be first #
######################################

all: ${TARGETS}


##################
# compile C code #
##################

.c.o:
	${CC} ${CFLAGS} -c $*.c

datesub: datesub.o
	${CC} ${LDFLAGS} datesub.o -o $@

gene: gene.o rand.o learn.o stats.o utility.o
	${CC} ${CFLAGS} ${LDFLAGS} gene.o rand.o learn.o stats.o utility.o -lm -o $@

histplot: histplot.o utility.o
	${CC} ${LDFLAGS} histplot.o utility.o -o $@

player: ${OBJS}
	${CC} ${LDFLAGS} ${OBJS} -lm ${LIBS} -o $@

rgmplot: rgmplot.o utility.o
	${CC} ${LDFLAGS} rgmplot.o utility.o -o $@

rogomatic: setup.o findscore.o scorefile.o utility.o
	${CC} ${LDFLAGS} setup.o findscore.o scorefile.o utility.o -o $@


#################################################################
# other targets that are not automatically built, not installed #
#################################################################

anim: anim.o utility.o
	${CC} anim.o utility.o ${LIBS} -o $@

genetest: genetest.o learn.o rand.o stats.o utility.o
	${CC} genetest.o learn.o rand.o stats.o utility.o -lm ${LIBS} -o $@

gplot: gplot.o
	${CC} -g gplot.o -lm ${LIBS} -o $@

testfind: testfind.o findscore.o utility.o
	${CC} ${LDFLAGS} testfind.o findscore.o utility.o ${LIBS} -o $@

tf: tf.o findscore.o
	${CC} ${LDFLAGS} tf.o findscore.o ${LIBS} -o $@

# NOTE: This rule is NOT part of the build of rogomatic documentation!
# 	We use this rule to form the rogomatic.cat.in file from the rogomatic.6.in file.
#
form_rogomatic_cat_in: rogomatic.6.in
	${RM} -f rogomatic.cat.in
	${GROFF} -Tascii -man rogomatic.6.in | LC_CTYPE=C ${SED} -e 's/.\x08//g' > rogomatic.cat.in


#####################
# constricted files #
#####################

datesub.c: datesub.l
	${RM} -f datesub.c
	${LEX} datesub.l
	${MV} -f lex.yy.c datesub.c


################################
# form rogomatic documentation #
################################

stddocs: rogomatic.6 rogomatic.cat

rogomatic.6: rogomatic.6.in
	${RM} -f $@
	LC_CTYPE=C ${SED} -e 's;${AT}XYZZY${AT};XXX_SOME_STRING_TBD_XXX;' \
			  rogomatic.6.in > rogomatic.6

rogomatic.cat: rogomatic.6 rogomatic.cat.in
	${RM} -f $@
	@-if test "x${GROFF}" != "x" -a "x${SED}" != "x" ; then \
	    echo "${GROFF} -Tascii -man rogomatic.6 | LC_CTYPE=C ${SED} -e 's/.\x08//g' > $@" ; \
	    ${GROFF} -Tascii -man rogomatic.6 | LC_CTYPE=C ${SED} -e 's/.\x08//g' > $@ ; \
	elif test "x${NROFF}" != "x" -a "x${TBL}" != "x" -a "x${COLCRT}" != "x" ; then \
	    echo "${NROFF} -man rogomatic.6 | ${COLCRT} - > $@" ; \
	    ${NROFF} -man rogomatic.6 | ${COLCRT} - > $@ ; \
        else \
	    echo "LC_CTYPE=C ${SED} \
			      -e 's;${AT}XYZZY${AT};XXX_SOME_STRING_TBD_XXX;' \
			      rogomatic.cat.in > $@" ; \
	    LC_CTYPE=C ${SED} \
			      -e 's;${AT}XYZZY${AT};XXX_SOME_STRING_TBD_XXX;' \
			      rogomatic.cat.in > $@ ; \
	fi


#################################################
# .PHONY list of rules that do not create files #
#################################################

.PHONY: all stddocs clean clobber install \
	backup clashes dist fluff print titler words \
	index depend


###################################
# standard Makefile utility rules #
###################################

clean:
	${RM} -f datesub.c lex.yy.c
	${RM} -f ${MISC_OBJS}
	${RM} -f ${OBJS}

clobber: clean
	${RM} -f tags index rogomatic.6 rogomatic.cat
	${RM} -f ${OTHER_TARGES}
	${RM} -f ${TARGETS}

install: ${TARGETS}
	${INSTALL} -d -m 0755 ${BINDIR}
	${INSTALL} -m 0755 ${TARGETS} ${BINDIR}

index: ${CFILES}
	${RM} -f index
	${CTAGS} -x ${C_SRC} > index

tags: ${SRC}
	${CTAGS} -w ${SRC}


###############################
# easy backward compatibility #
###############################

# These rules were referenced in the makefile of 2022 Nov 17
#
# We no longer support these rules.
#
# We provide these stub rules for backward compatibility.

backup:
	@echo dist rule disabled, use the GitHub repo: https://github.com/lcn2/rogomatic

clashes:
	@echo $@ disabled

dist:
	@echo dist rule disabled, use the GitHub repo: https://github.com/lcn2/rogomatic

fluff:
	@echo $@ disabled

print:
	@echo $@ disabled

titler:
	@echo $@ disabled

words:
	@echo $@ disabled


####################
# dependency rules #
####################

###
#
# Home grown make dependency rules.  Your system may not support
# or have the needed tools.  You can ignore this section.
#
# We will form a skeleton tree of *.c files containing only #include "foo.h"
# lines and .h files containing the same lines surrounded by multiple include
# prevention lines.  This allows us to build a static depend list that will
# satisfy all possible cpp symbol definition combinations.
#
###

depend: ${SRC}
	${Q} if [ -f ${MAKE_FILE}.bak ]; then \
	    echo "${MAKE_FILE}.bak exists, remove or move it"; \
	    exit 1; \
	else \
	    ${TRUE}; \
	fi
	${Q} if type -f ${MAKEDEPEND}; then \
	    ${TRUE}; \
	else \
	    echo "make depend failed: cannot find makedepend command: ${MAKEDEPEND}" 1>&2; \
	    exit 1; \
	fi
	${Q} ${RM} -rf skel
	${Q} ${MKDIR} -p skel
	-${Q} for i in ${C_SRC}; do \
	    ${SED} -n '/^#[	 ]*include[	 ]*"/p' "$$i" > "skel/$$i"; \
	done
	${Q} ${MKDIR} -p skel/custom
	-${Q} for i in ${H_SRC} modern_curses.h /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		tag="`echo $$i | ${SED} 's/[\.+,:]/_/g'`"; \
		echo "#if !defined($$tag)" > "skel/$$i"; \
		echo "#define $$tag" >> "skel/$$i"; \
		${SED} -n '/^#[	 ]*include[	 ]*"/p' "$$i" | \
		    LANG=C ${SORT} -u >> "skel/$$i"; \
		echo '#endif /* '"$$tag"' */' >> "skel/$$i"; \
	    fi; \
	done
	${Q} ${RM} -f skel/makedep.out
	${Q} echo top level skel formed
	${Q} echo forming dependency list
	${Q} :> skel/makedep.out
	${Q} cd skel; ${MAKEDEPEND} \
	    -w 1 -f makedep.out -- \
	    ${CFLAGS} -- \
	    ${SRC} modern_curses.h 2>/dev/null
	-${Q} for i in ${C_SRC} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
	      echo "$$i" | ${SED} 's/^\(.*\)\.c/\1.o: \1.c/'; \
	    fi; \
	done >> skel/makedep.out
	${Q} LANG=C ${SORT} -u skel/makedep.out -o skel/makedep.out
	${Q} echo dependency list formed
	${Q} echo forming new ${MAKE_FILE}
	${Q} ${RM} -f ${MAKE_FILE}.bak
	${Q} ${MV} ${MAKE_FILE} ${MAKE_FILE}.bak
	${Q} ${SED} -n '1,/^# DO NOT DELETE THIS LINE/p' \
		    ${MAKE_FILE}.bak > ${MAKE_FILE}
	${Q} ${GREP} -E -v '^#' skel/makedep.out >> ${MAKE_FILE}
	${Q} echo removing top level skel
	${Q} ${RM} -rf skel
	-${Q} if ${CMP} -s ${MAKE_FILE}.bak ${MAKE_FILE}; then \
	    echo 'top level ${MAKE_FILE} was already up to date'; \
	    echo 'restoring original ${MAKE_FILE}'; \
	    ${MV} -f ${MAKE_FILE}.bak ${MAKE_FILE}; \
	else \
	    echo 'old ${MAKE_FILE} is now ${MAKE_FILE}.bak'; \
	    echo 'new top level ${MAKE_FILE} formed'; \
	    echo 'try: diff -u ${MAKE_FILE}.bak ${MAKE_FILE}'; \
	fi

###
#
# make depend stuff
#
###

# DO NOT DELETE THIS LINE -- make depend depends on it.

arms.o: arms.c
arms.o: globals.h
arms.o: types.h
command.o: command.c
command.o: globals.h
command.o: types.h
database.o: database.c
database.o: globals.h
database.o: types.h
debug.o: debug.c
debug.o: globals.h
debug.o: install.h
debug.o: types.h
explore.o: explore.c
explore.o: globals.h
explore.o: types.h
findscore.o: findscore.c
findscore.o: globals.h
findscore.o: install.h
findscore.o: types.h
gene.o: gene.c
gene.o: globals.h
gene.o: install.h
gene.o: types.h
histplot.o: histplot.c
io.o: globals.h
io.o: install.h
io.o: io.c
io.o: termtokens.h
io.o: types.h
learn.o: globals.h
learn.o: learn.c
learn.o: types.h
ltm.o: globals.h
ltm.o: install.h
ltm.o: ltm.c
ltm.o: types.h
main.o: globals.h
main.o: install.h
main.o: main.c
main.o: termtokens.h
main.o: types.h
mess.o: globals.h
mess.o: mess.c
mess.o: types.h
monsters.o: globals.h
monsters.o: monsters.c
monsters.o: types.h
pack.o: globals.h
pack.o: pack.c
pack.o: types.h
rand.o: rand.c
replay.o: globals.h
replay.o: replay.c
replay.o: types.h
rgmplot.o: rgmplot.c
rooms.o: globals.h
rooms.o: rooms.c
rooms.o: types.h
scorefile.o: globals.h
scorefile.o: install.h
scorefile.o: scorefile.c
scorefile.o: types.h
search.o: globals.h
search.o: search.c
search.o: types.h
setup.o: globals.h
setup.o: install.h
setup.o: setup.c
setup.o: types.h
stats.o: stats.c
stats.o: types.h
strategy.o: globals.h
strategy.o: install.h
strategy.o: strategy.c
strategy.o: types.h
survival.o: globals.h
survival.o: survival.c
survival.o: types.h
tactics.o: globals.h
tactics.o: install.h
tactics.o: tactics.c
tactics.o: types.h
tf.o: globals.h
tf.o: tf.c
tf.o: types.h
things.o: globals.h
things.o: things.c
things.o: types.h
titlepage.o: globals.h
titlepage.o: titlepage.c
titlepage.o: types.h
utility.o: install.h
utility.o: utility.c
worth.o: globals.h
worth.o: types.h
worth.o: worth.c
