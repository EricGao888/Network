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
default: remote-command-srv
default: remote-command-cli

# To create the executable file count we need the object files
# countwords.o, counter.o, and scanner.o:
#
remote-command-srv: server.o
	gcc -o remote-command-srv server.o

remote-command-cli: client.o
	gcc -o remote-command-cli client.o

# To create the object file countwords.o, we need the source
# files countwords.c, scanner.h, and counter.h:
#
server.o: server.c
	gcc -c server.c

# To create the object file counter.o, we need the source files
# counter.c and counter.h:
#
client.o: client.c
	gcc -c client.c

# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
# files and *~ backup files:
#
clean: 
	rm -f server *.o *~
	rm -f client *.o *~