CFLAGS := -Wall -g -funsigned-char -I../../spamdyke

sendrecv: sendrecv.c ../../spamdyke/base64.h ../../spamdyke/base64.c ../../spamdyke/md5.h ../../spamdyke/md5.c Makefile
	gcc $(CFLAGS) -o sendrecv sendrecv.c ../../spamdyke/base64.c ../../spamdyke/md5.c -lssl -lcrypto

clean:
	rm -rf *.o sendrecv sendrecv.dSYM
