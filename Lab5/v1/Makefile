#
# This is an example Makefile for a countwords program.  This
# program uses both the scanner module and a counter module.
# Typing 'make' or 'make count' will create the executable file.
#

# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#
# for C++ define  CC = g++
# CC      = /usr/bin/gcc
# CFLAGS  = -ansi -pedantic -Wall -O2
# LIBPATH = -L.
# LDFLAGS = -o $(EXE) $(LIBPATH) $(LIBS)
# CFDEBUG = -ansi -pedantic -Wall -g -DDEBUG $(LDFLAGS)
# RM      = /bin/rm -f

# typing 'make' will invoke the first target entry in the file
# (in this case the default target entry)
# you can name this target entry anything, but "default" or "all"
# are the most commonly used names by convention
#
default: supergopher
default: minigopher
default: terve

supergopher: supergopher.o
	gcc -o supergopher supergopher.o

minigopher: minigopher.o
	gcc -o minigopher minigopher.o

terve: terve.o
	gcc -o terve terve.o

supergopher.o: supergopher.c supergopher.h
	gcc -c supergopher.c

minigopher.o: minigopher.c
	gcc -c minigopher.c

terve.o: terve.c
	gcc -c terve.c

# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
# files and *~ backup files:
#
clean:
	rm -f supergopher *.o *~
	rm -f minigopher *.o *~
	rm -f terve *.o *~