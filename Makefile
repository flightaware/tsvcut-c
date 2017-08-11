O=tsvcut.o
P=tsvcut

PREFIX=/usr/local

$P: $O
	cc $O -o $P

install: $P
	install -s $P $(PREFIX)/bin/$P
	install $P.1 $(PREFIX)/share/man/man1/$P.1
