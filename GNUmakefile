PROJECT=c-upm
VERSION=1.0.0
PREFIX=/usr/local
PROGS=
SOURCES  = openpwd/table.c openpwd/keyring.c openpwd/dmenu.c
HEADERS  = openpwd/table.h openpwd/keyring.h openpwd/dmenu.h openpwd/config.h
HEADERS += util/fgets2.h  util/rsalt.h  util/sexec.h
CC       =cc -Wall -std=c99

all: openpwd.bin
clean:
	rm -f openpwd.bin
install: openpwd.bin
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp openpwd.bin $(DESTDIR)$(PREFIX)/bin/openpwd

openpwd.bin: main.c $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ main.c $(SOURCES) $(LIBS)



## -- BLOCK:c --
clean: clean-c
clean-c:
	rm -f *.o
## -- BLOCK:c --
## -- BLOCK:man --
install: install-man
install-man:
	@mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	cp ./doc/openpwd.1 $(DESTDIR)$(PREFIX)/share/man/man1
## -- BLOCK:man --
