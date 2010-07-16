CFLAGS := -Wall -g -funsigned-char -I../../spamdyke

smtpdummy: smtpdummy.c Makefile
	gcc $(CFLAGS) -o smtpdummy smtpdummy.c

clean:
	rm -rf *.o smtpdummy smtpdummy.dSYM
