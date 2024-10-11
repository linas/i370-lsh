# Makefile for the lsh shell

# If you do not want to use the termcap library, 
# just delete the -DUSE_TERMCAP (and -ltermcap) and do a 
# make clean ; make
CC     = i370-ibm-linux-gcc
INCLUDES= -I../pdos-gitcode/pdpclib
CFLAGS = -Wall -O2 $(INCLUDES) -L ../pdos-gitcode/pdpclib
# LIBS   = -ltermcap
LIBS=

# The object files.
OBJ    = inp.o built.o parse.o map.o lex.o misc.o prompt.o exp.o alias.o main.o 

# Compile the shell (link the modules).
lsh: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o lsh $(LIBS)
#	-strip lsh

# Install the shell on you system (has to be done as root). This section
# requires cp, chmod, chown and would benefit from dialog, the groff 
# suite of programs, gzip, GNU install and echo.
install: lsh # <-delete this <lsh> if you do NOT want to recompile 
	-@dialog --title "LSH Installation" --msgbox "Ok to install lsh on your system ?" 5 50
	-@dialog --title "LSH Installation" --infobox "Installing Executable : \nlsh -> /bin/lsh" 4 50
	@cp     lsh      /bin/lsh
	@chown  root.bin /bin/lsh 
	@chmod  755      /bin/lsh
	-@dialog --title "LSH Installation" --infobox "Installing Configuration File : \nsample.etc.autoexec -> /etc/autoexec" 4 50
	@cp    sample.etc.autoexec /etc/autoexec
	@chown root.root           /etc/autoexec
	@chmod 644                 /etc/autoexec
	-@dialog --title "LSH Installation" --infobox "Installing Manual Page :\n/usr/man/man1/lsh" 4 50
	-@install -d     /usr/man/man1
	@cp    lsh.1     /usr/man/man1
	@chown root.root /usr/man/man1/lsh.1
	@chmod 644       /usr/man/man1/lsh.1
	-@dialog --title "LSH Installation" --infobox "Installing Preformatted Manual Page : \n/usr/man/preformat/cat1/lsh.1.gz" 4 50
	-@install -d     /usr/man/preformat/cat1
	-@groff -s -p -t -e -Tascii -mandoc lsh.1 | gzip -9 > /usr/man/preformat/cat1/lsh.1.gz
	-@chown root.man /usr/man/preformat/cat1/lsh.1.gz
	-@chmod 644      /usr/man/preformat/cat1/lsh.1.gz
	-@dialog --title "Installation Complete" --infobox "Remember to add a /bin/lsh\nentry to the /etc/shells file\nif your chsh or ftpd require that.\n\nFor a quick start try typing :\nlsh -k /etc/autoexec" 8 50
	-@echo -e "\nDone !\a\n"

# Quiet install. Does not preformat page.
quiet: lsh # <-delete this <lsh> if you do NOT want to recompile 
	install -m 755 -g bin -o root lsh /bin/lsh
	install -m 644 -o root sample.etc.autoexec /etc/autoexec
	install -m 644 -o root lsh.1 /usr/man/man1

# Compile the modules.
main.o: main.c
	$(CC) $(CFLAGS) -c main.c

inp.o: inp.c
	$(CC) $(CFLAGS) -c inp.c

lex.o: lex.c
	$(CC) $(CFLAGS) -c lex.c

map.o: map.c
	$(CC) $(CFLAGS) -c map.c

alias.o: alias.c
	$(CC) $(CFLAGS) -c alias.c

misc.o: misc.c
	$(CC) $(CFLAGS) -c misc.c

built.o: built.c
	$(CC) $(CFLAGS) -c built.c

parse.o: parse.c
	$(CC) $(CFLAGS) -c parse.c

exp.o: exp.c
	$(CC) $(CFLAGS) -c exp.c

prompt.o: prompt.c
	$(CC) $(CFLAGS) -c prompt.c

# Delete the object files.
clean: 
	-rm -f *.o
