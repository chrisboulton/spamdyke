CFLAGS := -Wall -O2 -funsigned-char -I../../spamdyke -I../../utils

all: smtpauth_plain smtpauth_login smtpauth_crammd5

smtpauth_plain: smtpauth_plain.c ../../spamdyke/base64.h ../../spamdyke/base64.c Makefile
	gcc $(CFLAGS) -o smtpauth_plain smtpauth_plain.c ../../spamdyke/base64.c

smtpauth_login: smtpauth_login.c ../../spamdyke/base64.h ../../spamdyke/base64.c Makefile
	gcc $(CFLAGS) -o smtpauth_login smtpauth_login.c ../../spamdyke/base64.c

smtpauth_crammd5: smtpauth_crammd5.c ../../spamdyke/base64.h ../../spamdyke/base64.c ../../spamdyke/md5.h ../../spamdyke/md5.c Makefile
	gcc $(CFLAGS) -o smtpauth_crammd5 smtpauth_crammd5.c ../../spamdyke/base64.c ../../spamdyke/md5.c

clean:
	rm -rf *.o smtpauth_plain smtpauth_plain.dSYM smtpauth_login smtpauth_login.dSYM smtpauth_crammd5 smtpauth_crammd5.dSYM

love:
	@echo Not war?
