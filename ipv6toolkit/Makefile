#
# SI6 Networks' IPv6 toolkit Makefile
#
# Notes to package developers:
#
# By default, binaries will be installed in /usr/local/bin, manual pages in
# /usr/local/man, data files in /usr/local/share/ipv6toolkit, and configuration
# files in /etc
#
# The path of the binaries and data files can be overriden by setting "PREFIX"
# variable accordingly. The path of the manual pages can be overriden by setting
# the MANPREFIX variable. Typically, packages will set these variables as follows:
#
# PREFIX=/usr
# MANPREFIX=/usr/share
#
# Finally, please note that this makefile supports the DESTDIR variable, as 
# typically employed by package developers.


CC?=gcc
CFLAGS+= -Wall -Wno-address-of-packed-member -Wno-missing-braces
LDFLAGS+= -lpcap -lm

ifndef PREFIX
	PREFIX=/usr/local
	ifndef MANPREFIX
		MANPREFIX=/usr/local
	endif
else
	ifndef MANPREFIX
		MANPREFIX=/usr/share
	endif
endif


ETCPATH= $(DESTDIR)/etc
MANPATH= $(DESTDIR)$(MANPREFIX)/man
BINPATH= $(DESTDIR)$(PREFIX)/bin
SRCPATH= tools

BINTOOLS= addr6
TOOLS= $(BINTOOLS)
LIBS= libipv6.o

all: $(TOOLS)

addr6: $(SRCPATH)/addr6.c $(SRCPATH)/addr6.h $(SRCPATH)/ipv6toolkit.h $(LIBS) $(SRCPATH)/libipv6.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o addr6 $(SRCPATH)/addr6.c $(LIBS) $(LDFLAGS) 

libipv6.o: $(SRCPATH)/libipv6.c $(SRCPATH)/libipv6.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o libipv6.o $(SRCPATH)/libipv6.c

clean: 
	rm -f $(TOOLS) $(LIBS)

install: all
	# Install the binaries
	install -m0755 -d $(BINPATH)
	install -m0755 $(BINTOOLS) $(BINPATH)

uninstall:
	# Remove the binaries
	rm -f $(BINPATH)/addr6

