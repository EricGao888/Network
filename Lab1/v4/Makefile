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
default: fifoser-v3
default: fifocli-v3	

# To create the executable file count we need the object files
# countwords.o, counter.o, and scanner.o:
#
fifoser-v3: fifoser-v3.o 
	gcc -o fifoser-v3 fifoser-v3.o 
#     $(CC) $(CFLAGS) -o fifoser-v3.o 

fifocli-v3: fifocli-v3.o 
	gcc -o fifocli-v3 fifocli-v3.o 
#     $(CC) $(CFLAGS) -o fifocli-v3.o 

# To create the object file countwords.o, we need the source
# files countwords.c, scanner.h, and counter.h:
#
fifoser-v3.o: fifoser-v3.c 
	gcc -c fifoser-v3.c
#     $(CC) $(CFLAGS) -c fifoser-v3.c 

# To create the object file counter.o, we need the source files
# counter.c and counter.h:
#
fifocli-v3.o: fifocli-v3.c 
	gcc -c fifocli-v3.c
#     $(CC) $(CFLAGS) -c fifocli-v3.c

# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
# files and *~ backup files:
#
clean: 
	rm -f fifoser-v3 *.o *~
	rm -f fifocli-v3 *.o *~