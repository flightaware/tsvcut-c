O=tsvcut.o
P=tsvcut
CFLAGS=-O2

PREFIX    ?=    /usr/local

$P: $O
	cc $(CFLAGS) $O -o $P

install: $P
	install -D -s $P $(PREFIX)/bin/$P
	install -D -d $(PREFIX)/share/man/man1
	install -D $P.1 $(PREFIX)/share/man/man1/$P.1

clean:
	rm -f tsvcut
