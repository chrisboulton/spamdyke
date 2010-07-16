If your spamdyke installation is broken or malfunctioning, the test scripts in
this directory won't help you.  Look at the files in the "documentation"
directory instead or read them on the spamdyke website:
        http://www.spamdyke.org/
If you still can't find the answers you need, ask for help on the spamdyke-users
mailing list.

These scripts are intended to be run by spamdyke developers, in order to verify
new versions of spamdyke contain no bugs.  They have no value to users or server
administrators.





!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
WARNING!WARNING!WARNING!WARNING!WARNING!WARNING!WARNING!WARNING!WARNING!WARNING!
 DANGER!DANGER!DANGER!DANGER!DANGER!DANGER!DANGER!DANGER!DANGER!DANGER!DANGER!
IMPORTANT!IMPORTANT!IMPORTANT!IMPORTANT!IMPORTANT!IMPORTANT!IMPORTANT!IMPORTANT!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

DO _NOT_ RUN THESE TEST SCRIPTS ON A PRODUCTION SERVER UNLESS YOU ARE
_ABSOLUTELY_, _POSITIVELY_, _100%_CERTAIN_ YOU _COMPLETELY_ UNDERSTAND HOW
THEY WORK.  SOME OF THESE SCRIPTS MODIFY IMPORTANT SYSTEM FILES AND CAN RENDER
YOUR SERVER NON-FUNCTIONAL.

THIS IS NOT A JOKE.  YOU HAVE BEEN WARNED.

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!





Still reading?  The test scripts are intended to test one feature, option or bug
each, (this is why there are so many).  Each directory contains one test and any
extra files it needs.  The test script in each directory is named "run.sh".
Those scripts are executed by the master "run" script in this (parent)
directory.  They can't be (easily) run individually.

NOTE: Tests with "patched" in their names will only execute correctly on servers
where /var/qmail/bin/qmail-smtpd has been patched to provide TLS and SMTP AUTH
support.  Tests with "unpatched" in their names will only execute correctly on
servers where /var/qmail/bin/qmail-smtpd has _not_ been patched (and thus does
not provide TLS or SMTP AUTH support).  Tests with "vpopmail" in their names
will only execute correctly on servers where vpopmail is installed with clear
password support (so /home/vpopmail/bin/vchkpw will process CRAM-MD5 input).

There are several programs in this directory:

addsecs/addsecs: A small C program that prints the current date and time plus
  a given number of seconds.  Used by a few scripts that test the graylist files
  and need to change the dates on those files (bash doesn't have a way to
  calculate arbitrary dates).

exitvalue/exitvalue: A small C program that generates exit values.  Used by
  a few scripts that test spamdyke's ability to catch different exit values from
  SMTP AUTH scripts.

run: The main script that runs all of the tests.  Usage below.

sendrecv/sendrecv: A rather large and complex C program that imitates a remote
  server sending data to spamdyke over the internet.  Its primary function is to
  read data from stdin (usually redirected from a text file) and send it slowly
  (one line at a time) and wait for SMTP responses.  It can exit when it sees a
  desired response.  It can kill spamdyke and exit when a timeout expires.  It
  can send portions of the input data in bursts (as some remote servers do).
  It can wait for the SMTP greeting banner or can send its data immediately.
  It can delay sending each line/burst of data for an arbitrary number of
  seconds.  It can answer CRAM-MD5 challenges with a given username/password.
  It can negotiate and carry out TLS transmissions.  It can deliberately corrupt
  TLS transmissions at any point.  It can encrypt all data with SMTPS.  Used
  by almost every test script.  Because much of the code (and all of the
  concepts) in sendrecv is copied from spamdyke, bugs in spamdyke have been
  known to get accidentally copied into sendrecv.  In at least one case, an
  identical bug in both programs managed to compensate for each other, so the
  test scripts were unable to detect the bug.

smtpauth/smtpauth_crammd5: A small C program that accepts a username, password
  and CRAM-MD5 challenge on the command line, then prints the CRAM-MD5 response
  to stdout.  Not used by any scripts; CRAM-MD5 testing is done by using the
  built-in features of sendrecv.

smtpauth/smtpauth_login: A small C program that accepts a username and password
  on the command line, then prints them to stdout, formatted for use with the
  SMTP AUTH LOGIN protocol.  Used by scripts that test SMTP AUTH LOGIN.

smtpauth/smtpauth_plain: A small C program that accepts a username and password
  on the command line, then prints them to stdout, formatted for use with the
  SMTP AUTH PLAIN protocol.  Used by scripts that test SMTP AUTH PLAIN.

smtpdummy/smtpdummy: A small C program that imitates an SMTP server, accepting
  input and giving SMTP response codes.  It can introduce arbitrary delays
  before sending specific responses.  Used by scripts that test spamdyke's
  behavior with slow/unresponsive qmail processes.

subrun: A symbolic link to "run", used by some of the tests that change user
  identities to test permissions.  Those scripts use "su" to execute the
  "subrun" script as another user.  The "run"/"subrun" script detects that it
  was called with the name "subrun" and modifies its behavior slightly.

................................................................................

The test scripts assume a GNU environment is available (e.g. Linux or
Free/Open/NetBSD).  They require GNU make, grep, find, sed, awk, wc and other
command line utilities.  If you're running a non-GNU system (e.g. Solaris),
the scripts won't work correctly.  Sorry.

To use the test scripts, you first must login as root (or "su").  Configure and
compile the spamdyke binary in the "spamdyke" directory with TLS support,
debugging output and excessive output.  Configure and compile the extra
utilities in the "utils" directory also.  It's not necessary to install any of
these binaries -- the test scripts will run them directly from the directories
where they were compiled.

Then you must set some environment variables.  If the "run" script is executed
with no arguments, it will output a usage message.  If the file
"/var/log/maillog" exists, it will search that file for suggested values for
these environment variables and print them after the usage message.  Since it
uses spamdyke log messages to do this, it obviously won't find anything unless
spamdyke is installed and running.

The variables are:

TESTSD_UNRESOLVABLE_RDNS_IP - IP address with an rDNS name that does not resolve
TESTSD_IP_IN_CC_RDNS - IP address with an rDNS name that contains the IP and a
  country code
TESTSD_IP_IN_RDNS_KEYWORD_IP - IP address with an rDNS name that contains the IP
  and a keyword
TESTSD_IP_IN_RDNS_KEYWORD - keyword found in TESTSD_IP_IN_RDNS_KEYWORD_IP's rDNS
  name
TESTSD_IP_IN_RDNS_PATTERN_IP - IP address with an rDNS name that contains the IP
  and a pattern
TESTSD_IP_IN_RDNS_PATTERN - pattern found in TESTSD_IP_IN_RDNS_PATTERN_IP's rDNS
  name
TESTSD_RDNS_IP - IP address with an rDNS name

To execute one or more tests, the "run" script needs a few values:
  1) An email address the local qmail server will accept for delivery.
  2) An account name and password to use for testing SMTP AUTH.  If vpopmail is
     installed, this should be a vpopmail account.  Otherwise it should be a
     system account (from /etc/passwd).  This does not have to be the same
     account as the email address in #1.
  3) A valid non-root system account name and password (from /etc/passwd) for
     testing both SMTP AUTH and permissions.  This does not have to be the same
     account used by qmail or related to the email addresses in #1 or #2.

To run all tests:
  cd spamdyke-x.y.z/tests
  ./run VALID_EMAIL_ADDRESS SMTPAUTH_ACCOUNTNAME SMTPAUTH_PASSWORD SYSTEM_ACCOUNTNAME SYSTEM_PASSWORD

To run a specific test or set of tests
  cd spamdyke-x.y.z/tests
  ./run VALID_EMAIL_ADDRESS SMTPAUTH_ACCOUNTNAME SMTPAUTH_PASSWORD SYSTEM_ACCOUNTNAME SYSTEM_PASSWORD [ TEST1 TEST2 TEST3 ... ]

To delete the test binaries and remove all temporary files:
  cd spamdyke-x.y.z/tests
  ./run clean

Good luck!
