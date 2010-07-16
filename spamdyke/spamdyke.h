/*
  spamdyke -- a filter for stopping spam at connection time.
  Copyright (C) 2010 Sam Clippinger (samc (at) silence (dot) org)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2 as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef SPAMDYKE_H
#define SPAMDYKE_H

#include "config.h"

#ifdef HAVE_STDINT_H

#include <stdint.h>

#else /* HAVE_STDINT_H */

#define INT32_MAX        2147483647

#endif /* HAVE_STDINT_H */

#include <sys/types.h>
#include <netinet/in.h>

#ifdef HAVE_GETOPT_H

#define _GNU_SOURCE
#include <getopt.h>

#else /* HAVE_GETOPT_H */

#include <unistd.h>

#endif /* HAVE_GETOPT_H */

#ifdef HAVE_LIBSSL

#include <openssl/ssl.h>

#define VERSION_TLS                     "+TLS"

#else /* HAVE_LIBSSL */

#define VERSION_TLS                     ""

#endif /* HAVE_LIBSSL */

#ifndef WITHOUT_CONFIG_TEST

#define VERSION_CONFIGTEST              "+CONFIGTEST"

#else /* WITHOUT_CONFIG_TEST */

#define VERSION_CONFIGTEST              ""

#endif /* WITHOUT_CONFIG_TEST */

#ifndef WITHOUT_DEBUG_OUTPUT

#define VERSION_DEBUG                   "+DEBUG"

#else /* WITHOUT_DEBUG_OUTPUT */

#define VERSION_DEBUG                   ""

#endif /* WITHOUT_DEBUG_OUTPUT */

#ifdef WITH_EXCESSIVE_OUTPUT

#define VERSION_EXCESSIVE               "+EXCESSIVE"

#else /* WITH_EXCESSIVE_OUTPUT */

#define VERSION_EXCESSIVE               ""

#endif /* WITH_EXCESSIVE_OUTPUT */

#define VERSION_STRING                  PACKAGE_VERSION VERSION_TLS VERSION_CONFIGTEST VERSION_DEBUG VERSION_EXCESSIVE

#define STRLEN(X)                       ((int)(sizeof(X) - 1))
#define _STRINGIFY(X)                   #X
#define STRINGIFY(X)                    _STRINGIFY(X)

#define DEFAULT_REMOTE_IP               "0.0.0.0"
#define DEFAULT_PATH                    "/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:/var/qmail/bin:/home/vpopmail/bin"
#define DEFAULT_NIHDNS_RESOLVER_FILENAME        "/etc/resolv.conf"
#define DEFAULT_NIHDNS_PORT             53
#define DEFAULT_TIMEOUT_NIHDNS_TOTAL_SECS       30
#define DEFAULT_NIHDNS_ATTEMPTS_PRIMARY 1
#define DEFAULT_NIHDNS_ATTEMPTS_TOTAL   3
#define DEFAULT_CONTROL_HOSTNAME        "/var/qmail/control/me"
#define TIMEOUT_CHECKPASSWORD_SECS      30
#define TIMEOUT_TLS_SHUTDOWN_SECS       5
#define TIMEOUT_COMMAND_SECS            30
#define TIMEOUT_IDLE_AFTER_QUIT_SECS    1200

#define MIN_SELECT_SECS_TIMEOUT         0
#define MIN_SELECT_USECS_TIMEOUT        500000
#define MAX_SELECT_SECS_TIMEOUT         2
#define MAX_SELECT_USECS_TIMEOUT        0
#define SELECT_SECS_NO_TIMEOUT          2
#define SELECT_USECS_NO_TIMEOUT         0

#define ENVIRONMENT_DELIMITER           '='
#define ENVIRONMENT_DELIMITER_STRING    "="
#define ENVIRONMENT_REMOTE_IP_TCPSERVER "TCPREMOTEIP"
#define ENVIRONMENT_REMOTE_IP_OLD_INETD "REMOTE_HOST"
#define ENVIRONMENT_REMOTE_IP           { ENVIRONMENT_REMOTE_IP_TCPSERVER, ENVIRONMENT_REMOTE_IP_OLD_INETD, NULL }
#define STRLEN_ENVIRONMENT_REMOTE_IP    { STRLEN(ENVIRONMENT_REMOTE_IP_TCPSERVER), STRLEN(ENVIRONMENT_REMOTE_IP_OLD_INETD), -1 }
#define ENVIRONMENT_REMOTE_NAME         "TCPREMOTEHOST"
#define ENVIRONMENT_REMOTE_INFO         "TCPREMOTEINFO"
#define ENVIRONMENT_LOCAL_PORT          "TCPLOCALPORT"
#define ENVIRONMENT_LOCAL_PORT_SMTP     "TCPLOCALPORT=25"
#define ENVIRONMENT_PATH                "PATH"
#define ENVIRONMENT_ALLOW_RELAY         "RELAYCLIENT"
#define ENVIRONMENT_HOSTNAME_TCPSERVER  "TCPLOCALHOST"
#define ENVIRONMENT_HOSTNAME_LINUX      "HOSTNAME"
#define ENVIRONMENT_HOSTNAME            (char *[]){ ENVIRONMENT_HOSTNAME_TCPSERVER, ENVIRONMENT_HOSTNAME_LINUX, NULL }
#define STRLEN_ENVIRONMENT_HOSTNAME     (int []){ STRLEN(ENVIRONMENT_HOSTNAME_TCPSERVER), STRLEN(ENVIRONMENT_HOSTNAME_LINUX), -1 }
#define ENVIRONMENT_RESOLV_OPTION       "RES_OPTIONS"
#define ENVIRONMENT_SMTPS               "SMTPS"

#define TCPRULES_INFO                   '@'
#define TCPRULES_ENVIRONMENT            ':'
#define TCPRULES_NAME                   '='
#define TCPRULES_ALLOW                  "allow"
#define TCPRULES_DENY                   "deny"
#define TCPRULES_DELIMITER              ','

#define COMMAND_LINE_SPACER             ','
#define COMMAND_LINE_TRUE               "1tTyY"
#define COMMAND_LINE_FALSE              "0fFnN"

#define STDIN_FD                        0
#define STDOUT_FD                       1
#define STDERR_FD                       2
#define CHECKPASSWORD_FD                3

#define CHAR_CR                         '\r'
#define CHAR_LF                         '\n'
#define STR_CRLF                        "\r\n"

#define MKDIR_MODE                      0700
#define CHMOD_MODE                      0600
#define DIR_CURRENT                     "."
#define DIR_PARENT                      ".."
#define DIR_DELIMITER                   '/'
#define DIR_DELIMITER_STR               "/"
#define USER_DELIMITER                  ":"

#define CONFIG_VALUE_CANCEL             "!!!"
#define CONFIG_VALUE_REMOVE             "!"

#define CONFIG_DIR_IP                   "_ip_"
#define CONFIG_DIR_NAME                 "_rdns_"
#define CONFIG_DIR_SENDER               "_sender_"
#define CONFIG_DIR_RECIPIENT            "_recipient_"
#define CONFIG_DIR_USERNAME             "_at_"

#define CONFIG_DIR_SEARCH_FIRST         0x00
#define CONFIG_DIR_SEARCH_ALL_IP        0x01
#define CONFIG_DIR_SEARCH_ALL_RDNS      0x02
#define CONFIG_DIR_SEARCH_ALL_SENDER    0x04
#define CONFIG_DIR_SEARCH_ALL_RECIPIENT 0x08

#define COMMENT_DELIMITER               '#'
#define VALUE_DELIMITER                 "="
#define RESOLVER_FILE_COMMENT_DELIMITER_1       '#'
#define RESOLVER_FILE_COMMENT_DELIMITER_2       ';'

#define MAX_ADDRESS                     511
#define MAX_PATH                        4095
#define MAX_BUF                         1023
#define MAX_NETWORK_BUF                 16383
#define MAX_FILE_BUF                    65535
#define MAX_FILE_LINES                  65536
#define MAX_POLICY                      100
#define MAX_RDNS                        29
#define MAX_IP                          15
#define MAX_CHECKPASSWORD               511
#define MAX_COMMAND_BUF                 4095
#define MAX_RAND_SEED                   65536
#define MAX_HOSTNAME                    127
#define MAX_BUF_SOCKET                  32768

#define MAX_NIHDNS_SERVERS              16
#define MAX_DNS_QUERIES                 16
#define MAX_DNS_PACKET_BYTES_UDP        512
#define MAX_DNS_PACKET_BYTES_TCP        65536
#define MAX_DNS_PACKETS                 6
#define MAX_DNS_PACKETS_PREFERRED       1

#define RDNS_SUFFIX                     ".in-addr.arpa"
#define MINVAL(a,b)                     ({ typeof (a) _a = (a); typeof (b) _b = (b); _a < _b ? _a : _b; })
#define MAXVAL(a,b)                     ({ typeof (a) _a = (a); typeof (b) _b = (b); _a > _b ? _a : _b; })

#define BATV_PREFIX                     "prvs="

#define MD5_IPAD_BYTE                   0x36
#define MD5_OPAD_BYTE                   0x5C

#define USAGE_LINE_WRAP                 80
#define USAGE_LINE_INDENT               "  "
#define USAGE_MESSAGE_HEADER            "spamdyke " VERSION_STRING " (C)2010 Sam Clippinger, " PACKAGE_BUGREPORT "\nhttp://www.spamdyke.org/\n\n"
#define USAGE_MESSAGE_USAGE             "USAGE: spamdyke [ OPTIONS ] [ -- ] qmail_smtpd_command [ qmail_smtpd_arguments ]\n"
#define USAGE_MESSAGE_INTEGER_RANGE     "%s must be between (or equal to) %d and %d.\n"
#define USAGE_MESSAGE_NAME_VALUES       "%s"
#define USAGE_MESSAGE_NAME_VALUE_DELIMITER      " | "
#define USAGE_MESSAGE_OPTIONAL_SHORT    "No spaces are allowed between '%c' and %s.\n"
#define USAGE_MESSAGE_OPTIONAL_LONG     "No spaces are allowed and an equals sign is required between %s and %s.\n"
#define USAGE_MESSAGE_ARRAY             "%s may be used multiple times.\n"
#define USAGE_MESSAGE_SINGLETON         "%s may only be used once.\n"
#define USAGE_MESSAGE_FOOTER_SHORT      "Use -h for an option summary or see README.html for complete option details.\n\n"
#define USAGE_MESSAGE_FOOTER_LONG       "\nSee README.html for a complete explanation of these options.\n"

#define FILTER_MASK_PASS                0x03
/* PASS is implied in assignments if another flag is not specified. */
#define FILTER_FLAG_PASS                0x00
#define FILTER_FLAG_INTERCEPT           0x01
#define FILTER_FLAG_QUIT                0x02

#define FILTER_MASK_CHILD               0x04
/* CHILD_CONTINUE is implied in assignments if CHILD_QUIT is not specified. */
#define FILTER_FLAG_CHILD_CONTINUE      0x00
#define FILTER_FLAG_CHILD_QUIT          0x04

#define FILTER_MASK_AUTH                0x38
/* AUTH_NONE is implied in assignments if another flag is not specified. */
/* AUTH values allow multiple flags to be set simultaneously. */
#define FILTER_FLAG_AUTH_NONE           0x00
#define FILTER_FLAG_AUTH_ADD            0x08
#define FILTER_FLAG_AUTH_CAPTURE        0x10
#define FILTER_FLAG_AUTH_REMOVE         0x20

#define FILTER_MASK_TLS                 0xC0
/* TLS_NONE is implied in assignments if another flag is not specified. */
#define FILTER_FLAG_TLS_NONE            0x00
#define FILTER_FLAG_TLS_ADD             0x40
#define FILTER_FLAG_TLS_CAPTURE         0x80
#define FILTER_FLAG_TLS_REMOVE          0xC0

#define FILTER_MASK_RCPT                0x0100
/* RCPT_NONE is implied in assignments if another flag is not specified. */
#define FILTER_FLAG_RCPT_NONE           0x0000
#define FILTER_FLAG_RCPT_CAPTURE        0x0100

#define FILTER_MASK_CHILD_RESPONSE      0x0200
/* CHILD_RESPONSE_NEEDED is implied in assignments if another flag is not specified. */
#define FILTER_FLAG_CHILD_RESPONSE_NEEDED       0x0000
#define FILTER_FLAG_CHILD_RESPONSE_NOT_NEEDED   0x0200

/*
 * The values of these constants are significant.  set_config_value() and
 * filter_*() use them to decide if the filter action should be set by comparing
 * the current value to the new value.  If the new value is greater than the
 * current value, the filter action is set.  Otherwise, it is not.
 *
 * When FILTER_DECISION_TRANSIENT_DO_FILTER is set, transient_rejection must also be set.
 * When FILTER_DECISION_DO_FILTER is set, rejection must also be set.
 */
#define FILTER_DECISION_UNDECIDED               0
#define FILTER_DECISION_TRANSIENT_DO_FILTER     1
#define FILTER_DECISION_DO_FILTER               2
#define FILTER_DECISION_TRANSIENT_DO_NOT_FILTER 3
#define FILTER_DECISION_DO_NOT_FILTER           4
#define FILTER_DECISION_CONFIG_TEST             5
#define FILTER_DECISION_FORK_ERROR              6
#define FILTER_DECISION_ERROR                   7

/*
 * The values of these constants must be in ascending order.
 */
#define FILTER_GRACE_EXPIRED            -1
#define FILTER_GRACE_NONE               0
#define FILTER_GRACE_AFTER_FROM         1
#define FILTER_GRACE_AFTER_TO           2
#define FILTER_GRACE_AFTER_DATA         3

#define FILTER_LEVEL_NORMAL             1
#define FILTER_LEVEL_ALLOW_ALL          2
#define FILTER_LEVEL_REQUIRE_AUTH       3
#define FILTER_LEVEL_REJECT_ALL         4

#define GRAYLIST_LEVEL_NONE             0x01

#define GRAYLIST_LEVEL_MASK_BEHAVIOR    0x18
#define GRAYLIST_LEVEL_FLAG_ALWAYS      0x08
#define GRAYLIST_LEVEL_FLAG_ONLY        0x10

#define GRAYLIST_LEVEL_MASK_CREATION    0x06
#define GRAYLIST_LEVEL_FLAG_NO_CREATE   0x02
#define GRAYLIST_LEVEL_FLAG_CREATE      0x04

#define TLS_STATE_INACTIVE              0
#define TLS_STATE_ACTIVE_SPAMDYKE       1
#define TLS_STATE_ACTIVE_PASSTHROUGH    2

#define TLS_LEVEL_NONE                  1
#define TLS_LEVEL_PROTOCOL              2
#define TLS_LEVEL_SMTPS                 3

#define TLS_DESC_UNKNOWN                "(unknown)"
#define TLS_DESC_INACTIVE               "(none)"
#define TLS_DESC_PASSTHROUGH            "TLS_PASSTHROUGH"
#define TLS_DESC_SPAMDYKE_PROTOCOL      "TLS"
#define TLS_DESC_SPAMDYKE_SMTPS         "SSL"

#define LOCALHOST_IP                    "127.0.0.1"
#define LOCALHOST_OCTETS                { 127, 0, 0, 1 }
#define LOCALHOST_NAME                  "localhost"
#define MISSING_LOCAL_SERVER_NAME       "unknown.server.unknown.domain"

#define ALPHABET_FILENAME               "abcdefghijklmnopqrstuvwxyz0123456789@_-.ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define REPLACEMENT_FILENAME            '_'
#define ALPHABET_BASE64                 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
#define PAD_BASE64                      '='

#define REJECT_SEVERITY_NONE            "250 "
#define REJECT_SEVERITY_HELO            "220 "
#define REJECT_SEVERITY_QUIT            "221 "
#define REJECT_SEVERITY_AUTH_SUCCESS    "235 "
#define REJECT_SEVERITY_AUTH_CHALLENGE  "334 "
#define REJECT_SEVERITY_TEMPORARY       "421 "
#define REJECT_SEVERITY_AUTH_UNKNOWN    "504 "
#define REJECT_SEVERITY_AUTH_FAILURE    "535 "
#define REJECT_SEVERITY_PERMANENT       "554 "
#define REJECT_SEVERITY_TLS_SUCCESS     "220 "
#define REJECT_SEVERITY_TLS_FAILURE     "454 "
#define STRLEN_REJECT_SEVERITY          4
#define REJECT_CRLF                     "\r\n"

#define SMTP_HELO                       "HELO"
#define SMTP_EHLO                       "EHLO"
#define SMTP_AUTH                       "AUTH"
#define SMTP_TLS                        "STARTTLS"
#define SMTP_MAIL_FROM                  "MAIL FROM"
#define SMTP_RCPT_TO                    "RCPT TO"
#define SMTP_DATA                       "DATA"
#define SMTP_DATA_END                   "."
#define SMTP_QUIT                       "QUIT"
#define SMTP_RSET                       "RSET"

/* These constants must be in ascending order. */
#define SMTP_AUTH_LEVEL_MASK                            0x07
#define SMTP_AUTH_LEVEL_VALUE_NONE                      0x01
#define SMTP_AUTH_LEVEL_VALUE_OBSERVE                   0x02
#define SMTP_AUTH_LEVEL_VALUE_ON_DEMAND                 0x03
#define SMTP_AUTH_LEVEL_VALUE_ON_DEMAND_ENCRYPTED       0x04
#define SMTP_AUTH_LEVEL_VALUE_ALWAYS                    0x05
#define SMTP_AUTH_LEVEL_VALUE_ALWAYS_ENCRYPTED          0x06

/*
 * The integer value 0 must match the UNSET value because prepare_settings()
 * sets smtp_auth_level to 0 before processing the command line.  The extra
 * action for smtp-auth-command conditionally sets smtp_auth_level if it
 * matches the UNSET value.
 *
 * However, after the command line has been processed, smtp_auth_level is
 * set to a default if it wasn't set.  The action for smtp-auth-command
 * still needs to set smtp_auth_level from a configuration file, so UNSET
 * must also be valid there.
 */
#define SMTP_AUTH_SET_MASK                      0x08
#define SMTP_AUTH_SET_VALUE_UNSET               0x00
#define SMTP_AUTH_SET_VALUE_SET                 0x08

#define SMTP_AUTH_UNKNOWN               -1
#define SMTP_AUTH_LOGIN                 0
#define SMTP_AUTH_PLAIN                 1
#define SMTP_AUTH_CRAM_MD5              2
#define SMTP_AUTH_TYPE_LOGIN            "LOGIN"
#define SMTP_AUTH_TYPE_PLAIN            "PLAIN"
#define SMTP_AUTH_TYPE_CRAM_MD5         "CRAM-MD5"
#define SMTP_AUTH_TYPES                 (char *[]){ SMTP_AUTH_TYPE_LOGIN, SMTP_AUTH_TYPE_PLAIN, SMTP_AUTH_TYPE_CRAM_MD5, NULL }
#define STRLEN_SMTP_AUTH_TYPES          (int []){ STRLEN(SMTP_AUTH_TYPE_LOGIN), STRLEN(SMTP_AUTH_TYPE_PLAIN), STRLEN(SMTP_AUTH_TYPE_CRAM_MD5), -1 }
#define SMTP_AUTH_ENCRYPTION            (int []){ 0, 0, 1, -1 }

#define SMTP_AUTH_LOGIN_CHALLENGE_1             "VXNlcm5hbWU6"
#define SMTP_AUTH_LOGIN_CHALLENGE_2             "UGFzc3dvcmQ6"

#define SMTP_AUTH_ORIGIN_NONE           0
#define SMTP_AUTH_ORIGIN_SPAMDYKE       1
#define SMTP_AUTH_ORIGIN_CHILD          2

#define SMTP_AUTH_STATE_UNKNOWN                 -1
#define SMTP_AUTH_STATE_NONE                    0
#define SMTP_AUTH_STATE_CMD_SEEN                1
#define SMTP_AUTH_STATE_CHALLENGE_1_SENT        2
#define SMTP_AUTH_STATE_RESPONSE_1_SEEN         3
#define SMTP_AUTH_STATE_CHALLENGE_2_SENT        4
#define SMTP_AUTH_STATE_RESPONSE_2_SEEN         5
#define SMTP_AUTH_STATE_AUTHENTICATED           6

#define SMTP_CONTINUATION               '-'
#define SMTP_STR_CONTINUATION           "-"
#define SMTP_STR_DONE                   " "
#define SMTP_EHLO_SUCCESS               "250"
#define SMTP_EHLO_AUTH_CORRECT          "AUTH "
#define SMTP_EHLO_AUTH_INCORRECT        "AUTH="
#define SMTP_EHLO_TLS                   "STARTTLS"
#define SMTP_EHLO_AUTH_INSERT_ENCRYPTION        "AUTH LOGIN PLAIN CRAM-MD5\r\n"
#define SMTP_EHLO_AUTH_INSERT_CLEAR     "AUTH LOGIN PLAIN\r\n"
#define SMTP_EHLO_TLS_INSERT            "STARTTLS\r\n"
#define SMTP_EHLO_NOTHING_INSERT        "X-NOTHING\r\n"
#define SMTP_AUTH_CHALLENGE             "334"
#define SMTP_AUTH_SUCCESS               "235"
#define SMTP_AUTH_PROMPT                "3"
#define SMTP_AUTH_FAILURE               "5"
#define SMTP_TLS_SUCCESS                "220"
#define SMTP_TLS_REGREETING             "220 ESMTP\r\n"
#define SMTP_RCPT_SUCCESS               "250"

#define CHILD_QUIT                      ".\r\nQUIT\r\n"

#define SENDER_ADDRESS_NONE             "_none"
#define SENDER_DOMAIN_NONE              "_none"
#define SENDER_DOMAIN_NONE_TEMP         "___none"

#define SHORT_SUCCESS                   "ALLOWED"
#define SHORT_TLS_PASSTHROUGH           "TLS_ENCRYPTED"

#define ERROR_URL                       " See: "
#define ERROR_URL_DELIMITER_DYNAMIC     '='
#define ERROR_URL_DELIMITER_STATIC      "#"

struct rejection_data
  {
  int rejection_index;
  char *reject_severity;
  char *reject_message;
  int strlen_reject_message;
  char *short_reject_message;
  int append_policy;
  };

#define SUCCESS_AUTH                    { -1, REJECT_SEVERITY_AUTH_SUCCESS, "Proceed.", 8, SHORT_SUCCESS "_AUTHENTICATED", 0 }
#define SUCCESS_TLS                     { -2, REJECT_SEVERITY_TLS_SUCCESS, "Proceed.", 8, SHORT_SUCCESS "_TLS", 0 }

#define REJECTION_RCPT_TO               0
#define ERROR_RCPT_TO                   "Too many recipients. Try the remaining addresses again later."
#define REJECTION_DATA_RCPT_TO          { REJECTION_RCPT_TO, REJECT_SEVERITY_TEMPORARY, ERROR_RCPT_TO, STRLEN(ERROR_RCPT_TO), "DENIED_TOO_MANY_RECIPIENTS", 1 }

#define REJECTION_RCPT_TO_LOCAL         1
#define ERROR_RCPT_TO_LOCAL             "Improper recipient address. Try supplying a domain name."
#define REJECTION_DATA_RCPT_TO_LOCAL    { REJECTION_RCPT_TO_LOCAL, REJECT_SEVERITY_PERMANENT, ERROR_RCPT_TO_LOCAL, STRLEN(ERROR_RCPT_TO_LOCAL), "DENIED_UNQUALIFIED_RECIPIENT", 1 }

#define REJECTION_GRAYLISTED            2
#define ERROR_GRAYLISTED                "Your address has been graylisted. Try again later."
#define REJECTION_DATA_GRAYLISTED       { REJECTION_GRAYLISTED, REJECT_SEVERITY_TEMPORARY, ERROR_GRAYLISTED, STRLEN(ERROR_GRAYLISTED), "DENIED_GRAYLISTED", 1 }

#define REJECTION_RDNS_MISSING          3
#define ERROR_RDNS_MISSING              "Refused. You have no reverse DNS entry."
#define REJECTION_DATA_RDNS_MISSING     { REJECTION_RDNS_MISSING, REJECT_SEVERITY_TEMPORARY, ERROR_RDNS_MISSING, STRLEN(ERROR_RDNS_MISSING), "DENIED_RDNS_MISSING", 1 }

#define REJECTION_RDNS_RESOLVE          4
#define ERROR_RDNS_RESOLVE              "Refused. Your reverse DNS entry does not resolve."
#define REJECTION_DATA_RDNS_RESOLVE     { REJECTION_RDNS_RESOLVE, REJECT_SEVERITY_TEMPORARY, ERROR_RDNS_RESOLVE, STRLEN(ERROR_RDNS_RESOLVE), "DENIED_RDNS_RESOLVE", 1 }

#define REJECTION_IP_IN_NAME_CC         5
#define ERROR_IP_IN_NAME_CC             "Refused. Your reverse DNS entry contains your IP address and a country code."
#define REJECTION_DATA_IP_IN_NAME_CC    { REJECTION_IP_IN_NAME_CC, REJECT_SEVERITY_PERMANENT, ERROR_IP_IN_NAME_CC, STRLEN(ERROR_IP_IN_NAME_CC), "DENIED_IP_IN_CC_RDNS", 1 }

#define REJECTION_IP_IN_NAME            6
#define ERROR_IP_IN_NAME                "Refused. Your reverse DNS entry contains your IP address and a banned keyword."
#define REJECTION_DATA_IP_IN_NAME       { REJECTION_IP_IN_NAME, REJECT_SEVERITY_PERMANENT, ERROR_IP_IN_NAME, STRLEN(ERROR_IP_IN_NAME), "DENIED_IP_IN_RDNS", 1 }

#define REJECTION_EARLYTALKER           7
#define ERROR_EARLYTALKER               "Refused. You are not following the SMTP protocol."
#define REJECTION_DATA_EARLYTALKER      { REJECTION_EARLYTALKER, REJECT_SEVERITY_PERMANENT, ERROR_EARLYTALKER, STRLEN(ERROR_EARLYTALKER), "DENIED_EARLYTALKER", 1 }

#define REJECTION_BLACKLIST_NAME        8
#define ERROR_BLACKLIST_NAME            "Refused. Your domain name is blacklisted."
#define REJECTION_DATA_BLACKLIST_NAME   { REJECTION_BLACKLIST_NAME, REJECT_SEVERITY_PERMANENT, ERROR_BLACKLIST_NAME, STRLEN(ERROR_BLACKLIST_NAME), "DENIED_BLACKLIST_NAME", 1 }

#define REJECTION_BLACKLIST_IP          9
#define ERROR_BLACKLIST_IP              "Refused. Your IP address is blacklisted."
#define REJECTION_DATA_BLACKLIST_IP     { REJECTION_BLACKLIST_IP, REJECT_SEVERITY_PERMANENT, ERROR_BLACKLIST_IP, STRLEN(ERROR_BLACKLIST_IP), "DENIED_BLACKLIST_IP", 1 }

#define REJECTION_TIMEOUT               10
#define ERROR_TIMEOUT                   "Timeout. Talk faster next time."
#define REJECTION_DATA_TIMEOUT          { REJECTION_TIMEOUT, REJECT_SEVERITY_TEMPORARY, ERROR_TIMEOUT, STRLEN(ERROR_TIMEOUT), "TIMEOUT", 1 }

#define REJECTION_SENDER_BLACKLISTED    11
#define ERROR_SENDER_BLACKLISTED        "Refused. Your sender address has been blacklisted."
#define REJECTION_DATA_SENDER_BLACKLISTED       { REJECTION_SENDER_BLACKLISTED, REJECT_SEVERITY_PERMANENT, ERROR_SENDER_BLACKLISTED, STRLEN(ERROR_SENDER_BLACKLISTED), "DENIED_SENDER_BLACKLISTED", 1 }

#define REJECTION_RECIPIENT_BLACKLISTED 12
#define ERROR_RECIPIENT_BLACKLISTED             "Refused. Mail is not being accepted at this address."
#define REJECTION_DATA_RECIPIENT_BLACKLISTED    { REJECTION_RECIPIENT_BLACKLISTED, REJECT_SEVERITY_PERMANENT, ERROR_RECIPIENT_BLACKLISTED, STRLEN(ERROR_RECIPIENT_BLACKLISTED), "DENIED_RECIPIENT_BLACKLISTED", 1 }

#define REJECTION_SENDER_NO_MX          13
#define ERROR_SENDER_NO_MX              "Refused. The domain of your sender address has no mail exchanger (MX)."
#define REJECTION_DATA_SENDER_NO_MX     { REJECTION_SENDER_NO_MX, REJECT_SEVERITY_TEMPORARY, ERROR_SENDER_NO_MX, STRLEN(ERROR_SENDER_NO_MX), "DENIED_SENDER_NO_MX", 1 }

#define REJECTION_RBL                   14
#define ERROR_RBL                       "Refused. Your IP address is listed in the RBL at "
#define REJECTION_DATA_RBL              { REJECTION_RBL, REJECT_SEVERITY_PERMANENT, ERROR_RBL, STRLEN(ERROR_RBL), "DENIED_RBL_MATCH", 1 }

#define REJECTION_RHSBL                 15
#define ERROR_RHSBL                     "Refused. Your domain name is listed in the RHSBL at "
#define REJECTION_DATA_RHSBL            { REJECTION_RHSBL, REJECT_SEVERITY_PERMANENT, ERROR_RHSBL, STRLEN(ERROR_RHSBL), "DENIED_RHSBL_MATCH", 1 }

#define REJECTION_SMTP_AUTH_FAILURE             16
#define ERROR_SMTP_AUTH_FAILURE                 "Refused. Authentication failed."
#define REJECTION_DATA_SMTP_AUTH_FAILURE        { REJECTION_SMTP_AUTH_FAILURE, REJECT_SEVERITY_AUTH_FAILURE, ERROR_SMTP_AUTH_FAILURE, STRLEN(ERROR_SMTP_AUTH_FAILURE), "FAILED_AUTH", 0 }

#define REJECTION_SMTP_AUTH_UNKNOWN             17
#define ERROR_SMTP_AUTH_UNKNOWN                 "Refused. Unknown authentication method."
#define REJECTION_DATA_SMTP_AUTH_UNKNOWN        { REJECTION_SMTP_AUTH_UNKNOWN, REJECT_SEVERITY_AUTH_UNKNOWN, ERROR_SMTP_AUTH_UNKNOWN, STRLEN(ERROR_SMTP_AUTH_UNKNOWN), "UNKNOWN_AUTH", 0 }

#define REJECTION_ACCESS_DENIED                 18
#define ERROR_ACCESS_DENIED                     "Refused. Access is denied."
#define REJECTION_DATA_ACCESS_DENIED            { REJECTION_ACCESS_DENIED, REJECT_SEVERITY_PERMANENT, ERROR_ACCESS_DENIED, STRLEN(ERROR_ACCESS_DENIED), "DENIED_ACCESS_DENIED", 1 }

#define REJECTION_RELAYING_DENIED               19
#define ERROR_RELAYING_DENIED                   "Refused. Sending to remote addresses (relaying) is not allowed."
#define REJECTION_DATA_RELAYING_DENIED          { REJECTION_RELAYING_DENIED, REJECT_SEVERITY_PERMANENT, ERROR_RELAYING_DENIED, STRLEN(ERROR_RELAYING_DENIED), "DENIED_RELAYING", 1 }

#define REJECTION_OTHER                         20
#define ERROR_OTHER                             ""
#define REJECTION_DATA_OTHER                    { REJECTION_OTHER, REJECT_SEVERITY_TEMPORARY, ERROR_OTHER, STRLEN(ERROR_OTHER), "DENIED_OTHER", 1 }

#define REJECTION_ZERO_RECIPIENTS       21
#define ERROR_ZERO_RECIPIENTS           "Refused. You must specify at least one valid recipient."
#define REJECTION_DATA_ZERO_RECIPIENTS  { REJECTION_ZERO_RECIPIENTS, REJECT_SEVERITY_PERMANENT, ERROR_ZERO_RECIPIENTS, STRLEN(ERROR_ZERO_RECIPIENTS), "DENIED_ZERO_RECIPIENTS" }

#define REJECTION_AUTH_REQUIRED                 22
#define ERROR_AUTH_REQUIRED                     "Refused. Authentication is required to send mail."
#define REJECTION_DATA_AUTH_REQUIRED            { REJECTION_AUTH_REQUIRED, REJECT_SEVERITY_PERMANENT, ERROR_AUTH_REQUIRED, STRLEN(ERROR_AUTH_REQUIRED), "DENIED_AUTH_REQUIRED", 1 }

#define REJECTION_UNCONDITIONAL                 23
#define ERROR_UNCONDITIONAL                     "Refused. Mail is not being accepted."
#define REJECTION_DATA_UNCONDITIONAL            { REJECTION_UNCONDITIONAL, REJECT_SEVERITY_PERMANENT, ERROR_UNCONDITIONAL, STRLEN(ERROR_UNCONDITIONAL), "DENIED_REJECT_ALL", 1 }

#define REJECTION_IDENTICAL_FROM_TO             24
#define ERROR_IDENTICAL_FROM_TO                 "Refused. Identical sender and recipient addresses are not allowed."
#define REJECTION_DATA_IDENTICAL_FROM_TO        { REJECTION_IDENTICAL_FROM_TO, REJECT_SEVERITY_PERMANENT, ERROR_IDENTICAL_FROM_TO, STRLEN(ERROR_IDENTICAL_FROM_TO), "DENIED_IDENTICAL_SENDER_RECIPIENT", 1 }

#define FAILURE_TLS                     25
#define ERROR_FAILURE_TLS               "Failed to negotiate TLS connection."
#define FAILURE_DATA_TLS                { FAILURE_TLS, REJECT_SEVERITY_TLS_FAILURE, ERROR_FAILURE_TLS, STRLEN(ERROR_FAILURE_TLS), "FAILED_TLS", 0 }

#define REJECTION_DATA                  (struct rejection_data []){ \
                                        REJECTION_DATA_RCPT_TO, \
                                        REJECTION_DATA_RCPT_TO_LOCAL, \
                                        REJECTION_DATA_GRAYLISTED, \
                                        REJECTION_DATA_RDNS_MISSING, \
                                        REJECTION_DATA_RDNS_RESOLVE, \
                                        REJECTION_DATA_IP_IN_NAME_CC, \
                                        REJECTION_DATA_IP_IN_NAME, \
                                        REJECTION_DATA_EARLYTALKER, \
                                        REJECTION_DATA_BLACKLIST_NAME, \
                                        REJECTION_DATA_BLACKLIST_IP, \
                                        REJECTION_DATA_TIMEOUT, \
                                        REJECTION_DATA_SENDER_BLACKLISTED, \
                                        REJECTION_DATA_RECIPIENT_BLACKLISTED, \
                                        REJECTION_DATA_SENDER_NO_MX, \
                                        REJECTION_DATA_RBL, \
                                        REJECTION_DATA_RHSBL, \
                                        REJECTION_DATA_SMTP_AUTH_FAILURE, \
                                        REJECTION_DATA_SMTP_AUTH_UNKNOWN, \
                                        REJECTION_DATA_ACCESS_DENIED, \
                                        REJECTION_DATA_RELAYING_DENIED, \
                                        REJECTION_DATA_OTHER, \
                                        REJECTION_DATA_ZERO_RECIPIENTS, \
                                        REJECTION_DATA_AUTH_REQUIRED, \
                                        REJECTION_DATA_UNCONDITIONAL, \
                                        REJECTION_DATA_IDENTICAL_FROM_TO, \
                                        FAILURE_DATA_TLS \
                                        }

#define LOG_USE_CONFIG_TEST             0x01
#define LOG_USE_STDERR                  0x02
#define LOG_USE_SYSLOG                  0x04

/*
 * Log levels must be listed in ascending order from least output to most.
 *
 * NONE: No logging at all.
 *
 * ERROR: Critical errors only, including low memory, network errors (not
 * including protocol errors), filesystem permission errors,
 * configuration-related errors and config-test errors
 *
 * INFO: Traffic logging, config-test success and info messages
 *
 * VERBOSE: Non-critical errors, including network errors caused by the remote
 * host, protocol errors, config-test status messages and child process error
 * messages
 *
 * DEBUG: High-level debugging output to show processing path
 *
 * EXCESSIVE: Low-level debugging output to processing progress and data
 */
#define LOG_LEVEL_NONE                  1
#define LOG_LEVEL_ERROR                 2
#define LOG_LEVEL_INFO                  3
#define LOG_LEVEL_VERBOSE               4
#define LOG_LEVEL_DEBUG                 5
#define LOG_LEVEL_EXCESSIVE             6

#define NIHDNS_LEVEL_NONE               1
#define NIHDNS_LEVEL_NORMAL             2
#define NIHDNS_LEVEL_AGGRESSIVE         3

#define NIHDNS_TYPE_A                   1
#define NIHDNS_TYPE_CNAME               5
#define NIHDNS_TYPE_MX                  15
#define NIHDNS_TYPE_NS                  2
#define NIHDNS_TYPE_PTR                 12
#define NIHDNS_TYPE_SOA                 6
#define NIHDNS_TYPE_TXT                 16
#define NIHDNS_TYPE_ANY                 255

#define NIHDNS_TCP_NONE                 1
#define NIHDNS_TCP_NORMAL               2

#define NIHDNS_SPOOF_ACCEPT_ALL         1
#define NIHDNS_SPOOF_ACCEPT_SAME_IP     2
#define NIHDNS_SPOOF_ACCEPT_SAME_PORT   3
#define NIHDNS_SPOOF_REJECT             4

#define NIHDNS_GETINT16(buffer)         (uint16_t)((buffer[0] << 8) | buffer[1])
#define NIHDNS_GETINT32(buffer)         (uint32_t)((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3])

#define NIHDNS_RESOLV_NAMESERVER        "nameserver"
#define NIHDNS_RESOLV_PORT              "port"
#define NIHDNS_RESOLV_TIMEOUT           "timeout"
#define NIHDNS_RESOLV_OPTIONS           "options"
#define NIHDNS_RESOLV_OPTION_TIMEOUT    "timeout:"

/*
 * The numeric order of these values is significant.  Any value greater than or
 * equal to RELAY_LEVEL_NORMAL will set the RELAYCLIENT environment variable
 * before qmail is started.
 */
#define RELAY_LEVEL_UNSET               0
#define RELAY_LEVEL_NO_RELAY            1
#define RELAY_LEVEL_NO_CHECK            2
#define RELAY_LEVEL_NORMAL              3
#define RELAY_LEVEL_ALLOW_ALL           4

#define SPAMDYKE_LOG_NONE(CURRENT_SETTINGS,FORMAT...)         ({ if (((CURRENT_SETTINGS) == NULL) || ((CURRENT_SETTINGS)->current_options == NULL) || ((CURRENT_SETTINGS)->current_options->log_dir != NULL) || ((CURRENT_SETTINGS)->current_options->log_level >= LOG_LEVEL_NONE)) spamdyke_log(CURRENT_SETTINGS,LOG_LEVEL_NONE,1,FORMAT); })
#define SPAMDYKE_RELOG_NONE(CURRENT_SETTINGS,FORMAT...)       ({ if (((CURRENT_SETTINGS) == NULL) || ((CURRENT_SETTINGS)->current_options == NULL) || ((CURRENT_SETTINGS)->current_options->log_dir != NULL) || ((CURRENT_SETTINGS)->current_options->log_level >= LOG_LEVEL_NONE)) spamdyke_log(CURRENT_SETTINGS,LOG_LEVEL_NONE,0,FORMAT); })
#define SPAMDYKE_LOG_ERROR(CURRENT_SETTINGS,FORMAT...)        ({ if (((CURRENT_SETTINGS) == NULL) || ((CURRENT_SETTINGS)->current_options == NULL) || ((CURRENT_SETTINGS)->current_options->log_dir != NULL) || ((CURRENT_SETTINGS)->current_options->log_level >= LOG_LEVEL_ERROR)) spamdyke_log(CURRENT_SETTINGS,LOG_LEVEL_ERROR,1,FORMAT); })
#define SPAMDYKE_RELOG_ERROR(CURRENT_SETTINGS,FORMAT...)      ({ if (((CURRENT_SETTINGS) == NULL) || ((CURRENT_SETTINGS)->current_options == NULL) || ((CURRENT_SETTINGS)->current_options->log_dir != NULL) || ((CURRENT_SETTINGS)->current_options->log_level >= LOG_LEVEL_ERROR)) spamdyke_log(CURRENT_SETTINGS,LOG_LEVEL_ERROR,0,FORMAT); })
#define SPAMDYKE_LOG_INFO(CURRENT_SETTINGS,FORMAT...)         ({ if (((CURRENT_SETTINGS) == NULL) || ((CURRENT_SETTINGS)->current_options == NULL) || ((CURRENT_SETTINGS)->current_options->log_dir != NULL) || ((CURRENT_SETTINGS)->current_options->log_level >= LOG_LEVEL_INFO)) spamdyke_log(CURRENT_SETTINGS,LOG_LEVEL_INFO,1,FORMAT); })
#define SPAMDYKE_RELOG_INFO(CURRENT_SETTINGS,FORMAT...)       ({ if (((CURRENT_SETTINGS) == NULL) || ((CURRENT_SETTINGS)->current_options == NULL) || ((CURRENT_SETTINGS)->current_options->log_dir != NULL) || ((CURRENT_SETTINGS)->current_options->log_level >= LOG_LEVEL_INFO)) spamdyke_log(CURRENT_SETTINGS,LOG_LEVEL_INFO,0,FORMAT); })
#define SPAMDYKE_LOG_VERBOSE(CURRENT_SETTINGS,FORMAT...)      ({ if (((CURRENT_SETTINGS) == NULL) || ((CURRENT_SETTINGS)->current_options == NULL) || ((CURRENT_SETTINGS)->current_options->log_dir != NULL) || ((CURRENT_SETTINGS)->current_options->log_level >= LOG_LEVEL_VERBOSE)) spamdyke_log(CURRENT_SETTINGS,LOG_LEVEL_VERBOSE,1,FORMAT); })
#define SPAMDYKE_RELOG_VERBOSE(CURRENT_SETTINGS,FORMAT...)    ({ if (((CURRENT_SETTINGS) == NULL) || ((CURRENT_SETTINGS)->current_options == NULL) || ((CURRENT_SETTINGS)->current_options->log_dir != NULL) || ((CURRENT_SETTINGS)->current_options->log_level >= LOG_LEVEL_VERBOSE)) spamdyke_log(CURRENT_SETTINGS,LOG_LEVEL_VERBOSE,0,FORMAT); })

#ifndef WITHOUT_DEBUG_OUTPUT

#define SPAMDYKE_LOG_DEBUG(CURRENT_SETTINGS,FORMAT,DATA...) ({ if (((CURRENT_SETTINGS) == NULL) || ((CURRENT_SETTINGS)->current_options == NULL) || ((CURRENT_SETTINGS)->current_options->log_dir != NULL) || ((CURRENT_SETTINGS)->current_options->log_level >= LOG_LEVEL_DEBUG)) spamdyke_log(CURRENT_SETTINGS,LOG_LEVEL_DEBUG,1,FORMAT,__func__,__FILE__,__LINE__,DATA); })
#define SPAMDYKE_RELOG_DEBUG(CURRENT_SETTINGS,FORMAT,DATA...)       ({ if (((CURRENT_SETTINGS) == NULL) || ((CURRENT_SETTINGS)->current_options == NULL) || ((CURRENT_SETTINGS)->current_options->log_dir != NULL) || ((CURRENT_SETTINGS)->current_options->log_level >= LOG_LEVEL_DEBUG)) spamdyke_log(CURRENT_SETTINGS,LOG_LEVEL_DEBUG,0,FORMAT,__func__,__FILE__,__LINE__,DATA); })

#else /* WITHOUT_DEBUG_OUTPUT */

#define SPAMDYKE_LOG_DEBUG(CURRENT_SETTINGS,FORMAT...)        ({ })
#define SPAMDYKE_RELOG_DEBUG(CURRENT_SETTINGS,FORMAT...)      ({ })

#endif /* WITHOUT_DEBUG_OUTPUT */

#ifdef WITH_EXCESSIVE_OUTPUT

#define SPAMDYKE_LOG_EXCESSIVE(CURRENT_SETTINGS,FORMAT,DATA...)    ({ if (((CURRENT_SETTINGS) == NULL) || ((CURRENT_SETTINGS)->current_options == NULL) || ((CURRENT_SETTINGS)->current_options->log_dir != NULL) || ((CURRENT_SETTINGS)->current_options->log_level >= LOG_LEVEL_EXCESSIVE)) spamdyke_log(CURRENT_SETTINGS,LOG_LEVEL_EXCESSIVE,1,FORMAT,__func__,__FILE__,__LINE__,DATA); })
#define SPAMDYKE_RELOG_EXCESSIVE(CURRENT_SETTINGS,FORMAT,DATA...)  ({ if (((CURRENT_SETTINGS) == NULL) || ((CURRENT_SETTINGS)->current_options == NULL) || ((CURRENT_SETTINGS)->current_options->log_dir != NULL) || ((CURRENT_SETTINGS)->current_options->log_level >= LOG_LEVEL_EXCESSIVE)) spamdyke_log(CURRENT_SETTINGS,LOG_LEVEL_EXCESSIVE,0,FORMAT,__func__,__FILE__,__LINE__,DATA); })

#else /* WITH_EXCESSIVE_OUTPUT */

#define SPAMDYKE_LOG_EXCESSIVE(CURRENT_SETTINGS,FORMAT...)    ({ })
#define SPAMDYKE_RELOG_EXCESSIVE(CURRENT_SETTINGS,FORMAT...)  ({ })

#endif /* WITH_EXCESSIVE_OUTPUT */

#define SYSLOG_IDENTIFIER               "spamdyke"

#define LOG_MISSING_DATA                "(unknown)"
#define LOG_MSG_TLS_NO_ERROR            "Operation failed but no error was reported by the SSL/TLS library"
#define LOG_MSG_TLS_ZERO_RETURN         "The connection was unexpectedly ended/closed"
#define LOG_MSG_TLS_RECALL              "The SSL/TLS library wants a function to be called again to complete after it's been recalled repeatedly. This shouldn't happen."
#define LOG_MSG_TLS_SYSCALL             "The operation failed due to an I/O error"
#define LOG_MSG_TLS_LIBRARY             "A protocol or library failure occurred"
#define LOG_MSG_TLS_EOF_FOUND           "Unexpected EOF found"

#define LOG_ERROR_GRAYLIST_FILE         "ERROR: cannot write to graylist file "
#define LOG_ERROR_MOVE_DESCRIPTORS      "ERROR: unable to move file descriptors: "
#define LOG_ERROR_FORK                  "ERROR: unable to fork: "
#define LOG_ERROR_PIPE                  "ERROR: unable to create pipe: "
#define LOG_ERROR_EXEC                  "ERROR: unable to execute child process "
#define LOG_ERROR_EXEC_FILE             "ERROR: unable to find executable "
#define LOG_ERROR_OPEN                  "ERROR: unable to open file "
#define LOG_ERROR_OPEN_KEYWORDS         "ERROR: unable to open keywords file "
#define LOG_ERROR_OPEN_SEARCH           "ERROR: unable to open file for searching "
#define LOG_ERROR_SOCKET_UDP            "ERROR: unable to create UDP socket: %s"
#define LOG_ERROR_SOCKET_TCP            "ERROR: unable to create TCP socket: %s"
#define LOG_ERROR_BIND                  "ERROR: unable to bind socket: %s"
#define LOG_ERROR_SETSOCKOPT            "ERROR: unable to set socket option: %s"
#define LOG_ERROR_SENDTO_INCOMPLETE     "ERROR: unable to send complete data packet, tried to send %d bytes, actually sent %d bytes"
#define LOG_ERROR_SENDTO                "ERROR: unable to send data packet, tried to send %d bytes: %s"
#define LOG_ERROR_STAT                  "ERROR: unable to stat() path "
#define LOG_ERROR_MKDIR                 "ERROR: unable to create directory "
#define LOG_ERROR_OPEN_LOG              "ERROR: unable to open traffic log file "
#define LOG_ERROR_MOVE                  "ERROR: unable to move file "
#define LOG_ERROR_UNLINK                "ERROR: unable to remove file "
#define LOG_ERROR_MALLOC                "ERROR: out of memory - unable to allocate %lu bytes"
#define LOG_ERROR_TLS_INIT              "ERROR: unable to initialize SSL/TLS library"
#define LOG_ERROR_TLS_CERTIFICATE       "ERROR: unable to load SSL/TLS certificate from file: "
#define LOG_ERROR_TLS_SETCIPHER         "ERROR: unable to set supported SSL/TLS ciphers: "
#define LOG_ERROR_TLS_PRIVATEKEY        "ERROR: unable to load or decrypt SSL/TLS private key from file: "
#define LOG_ERROR_TLS_CERT_CHECK        "ERROR: incorrect SSL/TLS private key password or SSL/TLS certificate/privatekey mismatch"
#define LOG_ERROR_TLS_ACCEPT            "ERROR: unable to start SSL/TLS connection"
#define LOG_ERROR_TLS_READ              "ERROR: unable to read from SSL/TLS stream"
#define LOG_ERROR_TLS_WRITE             "ERROR: unable to write to SSL/TLS stream"
#define LOG_ERROR_BAD_CONFIG            "ERROR: unparsable configuration option in file %s on line %d: %s"
#define LOG_ERROR_OPEN_CONFIG           "ERROR: unable to open config file %s: %s"
#define LOG_ERROR_RESOLV_NS_BAD         "ERROR: invalid/unparsable nameserver found: %s"
#define LOG_ERROR_RESOLV_NS_PORT_BAD    "ERROR: invalid/unparsable nameserver port number found, defaulting to %d instead: %s"
#define LOG_ERROR_RESOLV_NS_IGNORED     "ERROR: ignored nameserver (too many nameservers found): %s"
#define LOG_ERROR_RESOLV_PORT_BAD       "ERROR: invalid/unparsable default port found in file %s on line %d: %s"
#define LOG_ERROR_RESOLV_GLOBAL_TIMEOUT_BAD     "ERROR: invalid/unparsable total timeout found in file %s on line %d: %s"
#define LOG_ERROR_RESOLV_QUERY_TIMEOUT_BAD      "ERROR: invalid/unparsable query timeout found in file %s on line %d: %s"
#define LOG_ERROR_RESOLV_QUERY_TIMEOUT_BAD_ENV  "ERROR: invalid/unparsable query timeout found in environment variable %s: %s"
#define LOG_ERROR_GETUSER               "ERROR: unable to find user with name or ID %s"
#define LOG_ERROR_SETUSER               "ERROR: unable to set current user to %s(%d): %s"
#define LOG_ERROR_GETGROUP              "ERROR: unable to find group with name or ID %s"
#define LOG_ERROR_SETGROUP              "ERROR: unable to set current group to %s(%d): %s"
#define LOG_ERROR_FPRINTF_LOG           "ERROR: unable to write to log file %s: "
#define LOG_ERROR_FPRINTF_BYTES         "ERROR: unable to write %d bytes to file %s: "
#define LOG_ERROR_OPTION_LIST_ORDER     "ERROR: option_list is out of order: %s comes before %s"
#define LOG_ERROR_SHORT_OPTION_CONFLICT "ERROR: short option %c is used by at least two options: %s and %s"
#define LOG_ERROR_SMTPS_SUPPORT         "ERROR: unable to start SMTPS because TLS support is not available or an SSL/TLS certificate is not available; closing connection"
#define LOG_ERROR_LATE_EARLYTALKER      "ERROR: earlytalker filter cannot be activated after the start of the connection -- ignoring configuration option"
#define LOG_ERROR_NONBLOCK_INPUT        "ERROR: unable to set input socket to nonblocking: "
#define LOG_ERROR_NONBLOCK_OUTPUT       "ERROR: unable to set output socket to nonblocking: "
#define LOG_ERROR_NONBLOCK_DNS_UDP      "ERROR: unable to set DNS UDP socket to nonblocking: "
#define LOG_ERROR_NONBLOCK_DNS_TCP      "ERROR: unable to set DNS TCP socket to nonblocking: "
#define LOG_ERROR_UDP_SPOOF             "ERROR: UDP packet received from an unexpected server, could be a DNS spoofing attempt: IP %s, port %d"

#define LOG_VERBOSE_WRITE               "ERROR: unable to write %d bytes to file descriptor %d: "
#define LOG_VERBOSE_DNS_COMPRESSION     "ERROR: compressed DNS packet could not be decoded for %s; this could indicate a problem with the nameserver."
#define LOG_VERBOSE_DNS_RESPONSE        "ERROR: bad or invalid dns response to %s; this could indicate a problem with the name server."
#define LOG_VERBOSE_DNS_UNKNOWN_TYPE    "ERROR: DNS response for %s: expected type %s but received type %s"
#define LOG_VERBOSE_DNS_OVERSIZE        "ERROR: TCP DNS response for %s is %d total bytes, larger the maximum possible (%d bytes); something is very wrong here"
#define LOG_VERBOSE_DNS_CONNECT         "ERROR: unable to connect to DNS server %s:%d using TCP: "
#define LOG_VERBOSE_AUTH_FAILURE        "ERROR: authentication failure (bad username/password, vchkpw uses this to indicate SMTP access is not allowed): "
#define LOG_VERBOSE_AUTH_MISUSE         "ERROR: authentication misuse (no input given or no additional command path given, e.g. /bin/true): "
#define LOG_VERBOSE_AUTH_ERROR          "ERROR: authentication error (likely due to missing/unexecutable commands): "
#define LOG_VERBOSE_AUTH_VCHKPW_BAD_CHARS       "ERROR: authentication error %d (vchkpw uses this to indicate an unparsable email address): "
#define LOG_VERBOSE_AUTH_VCHKPW_UNKNOWN_USER    "ERROR: authentication error %d (vchkpw uses this to indicate an unknown username or authentication failure): "
#define LOG_VERBOSE_AUTH_VCHKPW_ENV_USER        "ERROR: authentication error %d (vchkpw uses this to indicate a failure to set the USER environment variable, possibly due to low memory): "
#define LOG_VERBOSE_AUTH_VCHKPW_ENV_HOME        "ERROR: authentication error %d (vchkpw uses this to indicate a failure to set the HOME environment variable, possibly due to low memory): "
#define LOG_VERBOSE_AUTH_VCHKPW_ENV_SHELL       "ERROR: authentication error %d (vchkpw uses this to indicate a failure to set the SHELL environment variable, possibly due to low memory): "
#define LOG_VERBOSE_AUTH_VCHKPW_ENV_VPOPUSER    "ERROR: authentication error %d (vchkpw uses this to indicate a failure to set the VPOPUSER environment variable, possibly due to low memory): "
#define LOG_VERBOSE_AUTH_VCHKPW_BAD_INPUT       "ERROR: authentication error %d (vchkpw uses this to indicate missing input on file descriptor 3): "
#define LOG_VERBOSE_AUTH_VCHKPW_NULL_USER       "ERROR: authentication error %d (vchkpw uses this to indicate an empty username): "
#define LOG_VERBOSE_AUTH_VCHKPW_NULL_PASSWORD   "ERROR: authentication error %d (vchkpw uses this to indicate an empty password): "
#define LOG_VERBOSE_AUTH_VCHKPW_HOME_DIR        "ERROR: authentication error %d (vchkpw uses this to indicate a failure to create a virtual user's home directory): "
#define LOG_VERBOSE_AUTH_VCHKPW_NO_PASSWORD     "ERROR: authentication error %d (vchkpw uses this to indicate a virtual user has no password): "
#define LOG_VERBOSE_AUTH_VCHKPW_UNKNOWN_SYSTEM_USER     "ERROR: authentication error %d (vchkpw uses this to indicate a failed username lookup in the system password file): "
#define LOG_VERBOSE_AUTH_VCHKPW_UNKNOWN_SYSTEM_SHADOW   "ERROR: authentication error %d (vchkpw uses this to indicate a failed username lookup in the system shadow file): "
#define LOG_VERBOSE_AUTH_VCHKPW_FAILURE_SYSTEM_USER     "ERROR: authentication error %d (vchkpw uses this to indicate a failed authentication attempt using the system password file): "
#define LOG_VERBOSE_AUTH_UNKNOWN        "ERROR: unknown authentication error code "
#define LOG_VERBOSE_AUTH_ABEND          "ERROR: authentication aborted abnormally: "
#define LOG_VERBOSE_COMMAND_ABEND       "ERROR: command aborted abnormally: "
#define LOG_VERBOSE_FILE_TOO_LONG       "ERROR: ignoring file content past line %d: "
#define LOG_VERBOSE_SMTPS_FAILURE       "ERROR: unable to start SMTPS due to a protocol failure; closing connection"
#define LOG_VERBOSE_FILTER_RDNS_MISSING "FILTER_RDNS_MISSING ip: %s"
#define LOG_VERBOSE_FILTER_IP_IN_RDNS_CC        "FILTER_IP_IN_CC_RDNS ip: %s rdns: %s"
#define LOG_VERBOSE_FILTER_RDNS_WHITELIST       "FILTER_WHITELIST_NAME ip: %s rdns: %s entry: %s"
#define LOG_VERBOSE_FILTER_RDNS_WHITELIST_FILE  "FILTER_WHITELIST_NAME ip: %s rdns: %s file: %s(%d)"
#define LOG_VERBOSE_FILTER_RDNS_WHITELIST_DIR   "FILTER_WHITELIST_NAME ip: %s rdns: %s path: %s"
#define LOG_VERBOSE_FILTER_RDNS_BLACKLIST       "FILTER_BLACKLIST_NAME ip: %s rdns: %s entry: %s"
#define LOG_VERBOSE_FILTER_RDNS_BLACKLIST_FILE  "FILTER_BLACKLIST_NAME ip: %s rdns: %s file: %s(%d)"
#define LOG_VERBOSE_FILTER_RDNS_BLACKLIST_DIR   "FILTER_BLACKLIST_NAME ip: %s rdns: %s path: %s"
#define LOG_VERBOSE_FILTER_IP_WHITELIST         "FILTER_WHITELIST_IP ip: %s entry: %s"
#define LOG_VERBOSE_FILTER_IP_WHITELIST_FILE    "FILTER_WHITELIST_IP ip: %s file: %s(%d)"
#define LOG_VERBOSE_FILTER_IP_BLACKLIST         "FILTER_BLACKLIST_IP ip: %s entry: %s"
#define LOG_VERBOSE_FILTER_IP_BLACKLIST_FILE    "FILTER_BLACKLIST_IP ip: %s file: %s(%d)"
#define LOG_VERBOSE_FILTER_IP_IN_RDNS_BLACKLIST "FILTER_IP_IN_RDNS_BLACKLIST ip: %s rdns: %s keyword: %s file: (none)"
#define LOG_VERBOSE_FILTER_IP_IN_RDNS_BLACKLIST_FILE    "FILTER_IP_IN_RDNS_BLACKLIST ip: %s rdns: %s keyword: %s file: %s(%d)"
#define LOG_VERBOSE_FILTER_IP_IN_RDNS_WHITELIST "FILTER_IP_IN_RDNS_WHITELIST ip: %s rdns: %s keyword: %s file: (none)"
#define LOG_VERBOSE_FILTER_IP_IN_RDNS_WHITELIST_FILE    "FILTER_IP_IN_RDNS_WHITELIST ip: %s rdns: %s keyword: %s file: %s(%d)"
#define LOG_VERBOSE_FILTER_RDNS_RESOLVE         "FILTER_RDNS_RESOLVE ip: %s rdns: %s"
#define LOG_VERBOSE_FILTER_DNS_RWL              "FILTER_RWL_MATCH ip: %s rwl: %s"
#define LOG_VERBOSE_FILTER_DNS_RHSWL            "FILTER_RHSWL_MATCH domain: %s rhswl: %s"
#define LOG_VERBOSE_FILTER_DNS_RBL              "FILTER_RBL_MATCH ip: %s rbl: %s"
#define LOG_VERBOSE_FILTER_DNS_RHSBL            "FILTER_RHSBL_MATCH domain: %s rhsbl: %s"
#define LOG_VERBOSE_FILTER_EARLYTALKER          "FILTER_EARLYTALKER delay: %d"
#define LOG_VERBOSE_FILTER_SENDER_WHITELIST     "FILTER_SENDER_WHITELIST sender: %s entry: %s"
#define LOG_VERBOSE_FILTER_SENDER_WHITELIST_FILE        "FILTER_SENDER_WHITELIST sender: %s file: %s(%d)"
#define LOG_VERBOSE_FILTER_SENDER_RHSWL         "FILTER_RHSWL_MATCH domain: %s rhswl: %s"
#define LOG_VERBOSE_FILTER_SENDER_BLACKLIST     "FILTER_SENDER_BLACKLIST sender: %s entry: %s"
#define LOG_VERBOSE_FILTER_SENDER_BLACKLIST_FILE        "FILTER_SENDER_BLACKLIST sender: %s file: %s(%d)"
#define LOG_VERBOSE_FILTER_SENDER_RHSBL         "FILTER_RHSBL_MATCH domain: %s rhsbl: %s"
#define LOG_VERBOSE_FILTER_SMTP_AUTH            "FILTER_AUTH_REQUIRED"
#define LOG_VERBOSE_FILTER_SENDER_MX            "FILTER_SENDER_NO_MX domain: %s"
#define LOG_VERBOSE_FILTER_RECIPIENT_WHITELIST  "FILTER_RECIPIENT_WHITELIST recipient: %s entry: %s"
#define LOG_VERBOSE_FILTER_RECIPIENT_WHITELIST_FILE     "FILTER_RECIPIENT_WHITELIST recipient: %s file: %s(%d)"
#define LOG_VERBOSE_FILTER_RECIPIENT_LOCAL      "FILTER_UNQUALIFIED_RECIPIENT recipient: %s"
#define LOG_VERBOSE_FILTER_RECIPIENT_BLACKLIST  "FILTER_RECIPIENT_BLACKLIST recipient: %s entry: %s"
#define LOG_VERBOSE_FILTER_RECIPIENT_BLACKLIST_FILE     "FILTER_RECIPIENT_BLACKLIST recipient: %s file: %s(%d)"
#define LOG_VERBOSE_FILTER_RELAY                "FILTER_RELAYING"
#define LOG_VERBOSE_FILTER_RECIPIENT_MAX        "FILTER_TOO_MANY_RECIPIENTS maximum: %d"
#define LOG_VERBOSE_FILTER_GRAYLIST             "FILTER_GRAYLISTED sender: %s recipient: %s path: %s"
#define LOG_VERBOSE_FILTER_ALLOW_ALL            "FILTER_ALLOW_ALL"
#define LOG_VERBOSE_FILTER_REJECT_ALL           "FILTER_REJECT_ALL"
#define LOG_VERBOSE_FILTER_OTHER_REJECTION      "FILTER_OTHER response: \"%.*s\""
#define LOG_VERBOSE_REMOTEIP_LOCALHOST          "ERROR: remote IP address missing, found text: \"%s\", using IP address %s"
#define LOG_VERBOSE_REMOTEIP_TEXT               "ERROR: remote IP address missing, found text: \"%s\", searching DNS for IP address"
#define LOG_VERBOSE_FILTER_IDENTICAL_FROM_TO    "FILTER_IDENTICAL_SENDER_RECIPIENT sender: %s recipient: %s"
#define LOG_VERBOSE_MX_IP               "ERROR: found IP address in MX record where only are legal: %s domain: %s"
#define LOG_VERBOSE_DNS_OVERSIZE_QUERY  "ERROR: unable to create DNS query packet in %d bytes, name: %s type %s"

#define LOG_DEBUG_AUTH_SUCCESS          "DEBUG(%s()@%s:%d): authentication successful: "
#define LOG_DEBUG_EXEC                  "DEBUG(%s()@%s:%d): executing command: %s"
#define LOG_DEBUG_EXEC_CHECKPASSWORD    "DEBUG(%s()@%s:%d): executing SMTP AUTH command %s for user: %s"
#define LOG_DEBUG_FILTER_RDNS_MISSING   "DEBUG(%s()@%s:%d): checking for missing rDNS; rdns: %s"
#define LOG_DEBUG_FILTER_IP_IN_RDNS_CC  "DEBUG(%s()@%s:%d): checking for IP in rDNS +country code; rdns: %s"
#define LOG_DEBUG_FILTER_RDNS_WHITELIST         "DEBUG(%s()@%s:%d): searching rDNS whitelist option(s); rdns: %s"
#define LOG_DEBUG_FILTER_RDNS_WHITELIST_FILE    "DEBUG(%s()@%s:%d): searching rDNS whitelist file(s); rdns: %s"
#define LOG_DEBUG_FILTER_RDNS_WHITELIST_DIR     "DEBUG(%s()@%s:%d): searching rDNS whitelist directory(ies); rdns: %s"
#define LOG_DEBUG_FILTER_RDNS_BLACKLIST         "DEBUG(%s()@%s:%d): searching rDNS blacklist option(s); rdns: %s"
#define LOG_DEBUG_FILTER_RDNS_BLACKLIST_FILE    "DEBUG(%s()@%s:%d): searching rDNS blacklist file(s); rdns: %s"
#define LOG_DEBUG_FILTER_RDNS_BLACKLIST_DIR     "DEBUG(%s()@%s:%d): searching rDNS blacklist directory(ies); rdns: %s"
#define LOG_DEBUG_FILTER_IP_WHITELIST   "DEBUG(%s()@%s:%d): searching IP whitelist file(s); ip: %s"
#define LOG_DEBUG_FILTER_IP_BLACKLIST   "DEBUG(%s()@%s:%d): searching IP blacklist file(s); ip: %s"
#define LOG_DEBUG_FILTER_IP_IN_RDNS_BLACKLIST   "DEBUG(%s()@%s:%d): checking for IP in rDNS +keyword(s) in blacklist file; ip: %s rdns: %s"
#define LOG_DEBUG_FILTER_IP_IN_RDNS_WHITELIST   "DEBUG(%s()@%s:%d): checking for IP in rDNS +keyword(s) in whitelist file; ip: %s rdns: %s"
#define LOG_DEBUG_FILTER_RDNS_RESOLVE   "DEBUG(%s()@%s:%d): checking rDNS resolution; rdns: %s"
#define LOG_DEBUG_FILTER_DNS_RWL        "DEBUG(%s()@%s:%d): checking DNS RWL(s); ip: %s"
#define LOG_DEBUG_FILTER_DNS_RHSWL      "DEBUG(%s()@%s:%d): checking rDNS RHSWL(s); rdns: %s"
#define LOG_DEBUG_FILTER_DNS_RBL        "DEBUG(%s()@%s:%d): checking DNS RBL(s); ip: %s"
#define LOG_DEBUG_FILTER_DNS_RHSBL      "DEBUG(%s()@%s:%d): checking rDNS RHSBL(s); rdns: %s"
#define LOG_DEBUG_FILTER_EARLYTALKER    "DEBUG(%s()@%s:%d): checking for earlytalker; delay: %d"
#define LOG_DEBUG_FILTER_SENDER_WHITELIST       "DEBUG(%s()@%s:%d): searching sender whitelist(s); sender: %s"
#define LOG_DEBUG_FILTER_SENDER_RHSWL           "DEBUG(%s()@%s:%d): checking sender domain RHSWL(s); domain: %s"
#define LOG_DEBUG_FILTER_SENDER_BLACKLIST       "DEBUG(%s()@%s:%d): searching sender blacklist(s); sender: %s"
#define LOG_DEBUG_FILTER_SENDER_RHSBL           "DEBUG(%s()@%s:%d): checking sender domain RHSBL(s); domain: %s"
#define LOG_DEBUG_FILTER_SMTP_AUTH              "DEBUG(%s()@%s:%d): checking for SMTP AUTH success; authenticated: %s"
#define LOG_DEBUG_FILTER_SENDER_MX              "DEBUG(%s()@%s:%d): checking for sender domain MX record; domain: %s"
#define LOG_DEBUG_FILTER_RECIPIENT_WHITELIST    "DEBUG(%s()@%s:%d): searching recipient whitelist(s); recipient: %s"
#define LOG_DEBUG_FILTER_RECIPIENT_LOCAL        "DEBUG(%s()@%s:%d): checking for unqualified recipient; recipient: %s"
#define LOG_DEBUG_FILTER_RECIPIENT_BLACKLIST    "DEBUG(%s()@%s:%d): searching recipient blacklist(s); recipient: %s"
#define LOG_DEBUG_FILTER_RELAY                  "DEBUG(%s()@%s:%d): checking relaying; relay-level: %d recipient: %s ip: %s rdns: %s local_recipient: %s relaying_allowed: %s"
#define LOG_DEBUG_FILTER_RECIPIENT_MAX          "DEBUG(%s()@%s:%d): checking maximum recipients; maximum: %d current: %d"
#define LOG_DEBUG_FILTER_GRAYLIST               "DEBUG(%s()@%s:%d): checking graylist; recipient: %s sender: %s"
#define LOG_DEBUG_CONFIG_SEARCH         "DEBUG(%s()@%s:%d): searching for config file or dir at %s"
#define LOG_DEBUG_CONFIG_SEARCH_DIR     "DEBUG(%s()@%s:%d): searching for config dir at %s"
#define LOG_DEBUG_CONFIG_FILE           "DEBUG(%s()@%s:%d): reading configuration file: %s"
#define LOG_DEBUG_NO_SETUSER            "DEBUG(%s()@%s:%d): no UID switch requested, running as: %s (%d)"
#define LOG_DEBUG_IDLE_RESET            "DEBUG(%s()@%s:%d): child process closed; resetting idle timeout from 0 to %d"
#define LOG_DEBUG_REMOTEIP_DNS_FOUND    "DEBUG(%s()@%s:%d): found remote IP address using DNS: %s"
#define LOG_DEBUG_REMOTEIP_DNS_NOT_FOUND        "DEBUG(%s()@%s:%d): no remote IP address found using DNS, using default: %s"
#define LOG_DEBUG_REMOTEIP_ENV_UPDATED  "DEBUG(%s()@%s:%d): updated environment with remote IP address: %s"
#define LOG_DEBUG_FILTER_IDENTICAL_FROM_TO      "DEBUG(%s()@%s:%d): comparing addresses; sender: %s recipient: %s"
#define LOG_DEBUG_ADDRESS_CONTROL_CHAR  "DEBUG(%s()@%s:%d): found unprintable control character in address at position %d, ASCII code %d"
#define LOG_DEBUG_ADDRESS_EMPTY_USERNAME        "DEBUG(%s()@%s:%d): unable to parse username from address: %s"
#define LOG_DEBUG_ADDRESS_EMPTY_DOMAIN  "DEBUG(%s()@%s:%d): unable to parse domain from address: %s"
#define LOG_DEBUG_FIND_ADDRESS          "DEBUG(%s()@%s:%d): found username: %s domain: %s"

#define LOG_DEBUGX_EXEC                 "EXCESSIVE(%s()@%s:%d): preparing to start child process: %s"
#define LOG_DEBUGX_DNS_QUERY            "EXCESSIVE(%s()@%s:%d): sending %d byte query (ID %d/%d) for %s(%s) to DNS server %s:%d (attempt %d)"
#define LOG_DEBUGX_DNS_RECEIVED         "EXCESSIVE(%s()@%s:%d): received DNS packet: %d bytes, ID %d/%d"
#define LOG_DEBUGX_DNS_RECEIVED_TYPE    "EXCESSIVE(%s()@%s:%d): received DNS response: %s, expected %s"
#define LOG_DEBUGX_DNS_TXT              "EXCESSIVE(%s()@%s:%d): found TXT record for %s: %.*s"
#define LOG_DEBUGX_DNS_A                "EXCESSIVE(%s()@%s:%d): found A record for %s: %d.%d.%d.%d"
#define LOG_DEBUGX_DNS_CNAME            "EXCESSIVE(%s()@%s:%d): found CNAME record for %s: %s"
#define LOG_DEBUGX_DNS_PTR              "EXCESSIVE(%s()@%s:%d): found PTR record for %s (%ld bytes): %.*s"
#define LOG_DEBUGX_DNS_MX               "EXCESSIVE(%s()@%s:%d): found MX record for %s: %d %s"
#define LOG_DEBUGX_DNS_NEGATIVE         "EXCESSIVE(%s()@%s:%d): found no records for %s"
#define LOG_DEBUGX_DOMAIN_DIR           "EXCESSIVE(%s()@%s:%d): searching for domain directory entry: %s"
#define LOG_DEBUGX_RESOLV_NS_LOAD       "EXCESSIVE(%s()@%s:%d): found nameserver at %s(%d): %s"
#define LOG_DEBUGX_RESOLV_NS_LOAD_DUPLICATE     "EXCESSIVE(%s()@%s:%d): discarded duplicate nameserver found at %s(%d): %s"
#define LOG_DEBUGX_RESOLV_NS            "EXCESSIVE(%s()@%s:%d): found nameserver: %s:%d"
#define LOG_DEBUGX_RESOLV_PORT          "EXCESSIVE(%s()@%s:%d): found resolver default port at %s(%d): %d"
#define LOG_DEBUGX_RESOLV_GLOBAL_TIMEOUT        "EXCESSIVE(%s()@%s:%d): found resolver global timeout at %s(%d): %d"
#define LOG_DEBUGX_RESOLV_QUERY_TIMEOUT "EXCESSIVE(%s()@%s:%d): found resolver query timeout at %s(%d): %d"
#define LOG_DEBUGX_RESOLV_QUERY_TIMEOUT_ENV     "EXCESSIVE(%s()@%s:%d): found resolver query timeout in environment variable %s: %d"
#define LOG_DEBUGX_RESOLV_NS_LOOPBACK   "EXCESSIVE(%s()@%s:%d): no nameservers found, using default server: %s:%d"
#define LOG_DEBUGX_RESOLV_IGNORED       "EXCESSIVE(%s()@%s:%d): ignored line at %s(%d): %s"
#define LOG_DEBUGX_SETUSER              "EXCESSIVE(%s()@%s:%d): set current user to %s(%d)."
#define LOG_DEBUGX_SETGROUP             "EXCESSIVE(%s()@%s:%d): set current group to %s(%d)."
#define LOG_DEBUGX_IP_IN_RDNS           "EXCESSIVE(%s()@%s:%d): searching for %.*s: %.*s"
#define LOG_DEBUGX_CHILD_EXIT_NORMAL    "EXCESSIVE(%s()@%s:%d): child process exited normally with return value %d"
#define LOG_DEBUGX_CHILD_EXIT_SIGNAL    "EXCESSIVE(%s()@%s:%d): child process exited/crashed due to receipt of signal %d"
#define LOG_DEBUGX_CHILD_EXIT_SIGNAL_CORE       "EXCESSIVE(%s()@%s:%d): child process exited/crashed due to receipt of signal %d and dumped core"
#define LOG_DEBUGX_CHILD_EXIT_STOPPED   "EXCESSIVE(%s()@%s:%d): child process is stopped by signal %d, probably by a debugger"
#define LOG_DEBUGX_CHILD_EXIT_STARTED   "EXCESSIVE(%s()@%s:%d): child process has resumed after being stopped, probably by a debugger"
#define LOG_DEBUGX_CHILD_EXIT_NONE      "EXCESSIVE(%s()@%s:%d): child process has not exited"
#define LOG_DEBUGX_REMOTE_IP_ENV        "EXCESSIVE(%s()@%s:%d): found remote IP address environment variable %s: %s"
#define LOG_DEBUGX_REMOTE_IP_DEFAULT    "EXCESSIVE(%s()@%s:%d): remote IP address not found in an environment variable, using default: %s"
#define LOG_DEBUGX_GRAYLIST_DOMAIN_FOUND        "EXCESSIVE(%s()@%s:%d): found existing domain directory for graylisting: %s"
#define LOG_DEBUGX_GRAYLIST_DOMAIN_CREATE       "EXCESSIVE(%s()@%s:%d): created domain directory for graylisting: %s"
#define LOG_DEBUGX_GRAYLIST_RECIPIENT_CREATE    "EXCESSIVE(%s()@%s:%d): created recipient directory for graylisting: %s"
#define LOG_DEBUGX_GRAYLIST_SENDER_CREATE       "EXCESSIVE(%s()@%s:%d): created sender directory for graylisting: %s"
#define LOG_DEBUGX_GRAYLIST_MOVE        "EXCESSIVE(%s()@%s:%d): converted graylist directory from old structure to new structure: %s to %s"
#define LOG_DEBUGX_TEST_GRAYLIST_DOMAIN_DIR     "EXCESSIVE(%s()@%s:%d): found graylist recipient domain directory: %s"
#define LOG_DEBUGX_TEST_GRAYLIST_USER_DIR       "EXCESSIVE(%s()@%s:%d): found graylist recipient user directory: %s"
#define LOG_DEBUGX_TEST_GRAYLIST_SENDER_DIR     "EXCESSIVE(%s()@%s:%d): found graylist sender domain directory: %s"
#define LOG_DEBUGX_TEST_GRAYLIST_SENDER_FILE    "EXCESSIVE(%s()@%s:%d): found graylist sender user file: %s"
#define LOG_DEBUGX_OPEN_FILE            "EXCESSIVE(%s()@%s:%d): opened file for reading: %s"
#define LOG_DEBUGX_READ_LINE            "EXCESSIVE(%s()@%s:%d): read %d bytes from %s, line %d: %s"
#define LOG_DEBUGX_CHILD_READ           "EXCESSIVE(%s()@%s:%d): read %d bytes from child input file descriptor %d, buffer contains %d bytes, current position is %d"
#define LOG_DEBUGX_CHILD_FD_EOF         "EXCESSIVE(%s()@%s:%d): child input file descriptor %d indicates EOF, buffer contains %d bytes, current position is %d"
#define LOG_DEBUGX_NETWORK_READ         "EXCESSIVE(%s()@%s:%d): read %d bytes from network input file descriptor %d, buffer contains %d bytes, current position is %d"
#define LOG_DEBUGX_NETWORK_FD_EOF       "EXCESSIVE(%s()@%s:%d): network input file descriptor %d indicates EOF, buffer contains %d bytes, current position is %d"
#define LOG_DEBUGX_CHILD_IN_CLOSE       "EXCESSIVE(%s()@%s:%d): child input file descriptor %d closed"
#define LOG_DEBUGX_CHILD_OUT_CLOSE      "EXCESSIVE(%s()@%s:%d): child output file descriptor %d closed"
#define LOG_DEBUGX_SET_VALUE_FROM_FILE  "EXCESSIVE(%s()@%s:%d): set configuration option %s from file %s, line %d: %s"
#define LOG_DEBUGX_FILTER_EARLYTALKER   "EXCESSIVE(%s()@%s:%d): found earlytalker"
#define LOG_DEBUGX_SMTP_AUTH_REPLACE    "EXCESSIVE(%s()@%s:%d): EHLO received; going to hide existing SMTP AUTH and add new SMTP AUTH"
#define LOG_DEBUGX_SMTP_AUTH_ADD        "EXCESSIVE(%s()@%s:%d): EHLO received; going to add SMTP AUTH"
#define LOG_DEBUGX_SMTP_AUTH_REMOVE     "EXCESSIVE(%s()@%s:%d): EHLO received; going to remove SMTP AUTH"
#define LOG_DEBUGX_TLS_ADD              "EXCESSIVE(%s()@%s:%d): EHLO received; going to add TLS"
#define LOG_DEBUGX_TLS_REMOVE           "EXCESSIVE(%s()@%s:%d): EHLO received; going to remove TLS"
#define LOG_DEBUGX_ENVIRONMENT_RELAY_FOUND      "EXCESSIVE(%s()@%s:%d): environment variable found to allow relaying: %s"
#define LOG_DEBUGX_ENVIRONMENT_RELAY_ADD        "EXCESSIVE(%s()@%s:%d): adding environment variable to allow relaying: %s"
#define LOG_DEBUGX_ENVIRONMENT_RELAY_ALLOWED    "EXCESSIVE(%s()@%s:%d): relaying allowed for this connection: %s"
#define LOG_DEBUGX_ENVIRONMENT_ADD      "EXCESSIVE(%s()@%s:%d): adding environment variable from %s: %.*s%.*s"
#define LOG_DEBUGX_ENVIRONMENT_LOCAL_PORT_FOUND "EXCESSIVE(%s()@%s:%d): environment variable found for local port: %s"
#define LOG_DEBUGX_ENVIRONMENT_LOCAL_PORT_SET   "EXCESSIVE(%s()@%s:%d): setting environment variable for local port: %s"
#define LOG_DEBUGX_ENVIRONMENT_SMTPS_REMOVE     "EXCESSIVE(%s()@%s:%d): removing environment variable for SMTPS: %s"
#define LOG_DEBUGX_ADDRESS_FOUND_WHOLE  "EXCESSIVE(%s()@%s:%d): found entire address (%d bytes): %.*s"
#define LOG_DEBUGX_ADDRESS_FOUND_USERNAME       "EXCESSIVE(%s()@%s:%d): found username in address (%d bytes): %.*s"
#define LOG_DEBUGX_ADDRESS_FOUND_DOMAIN "EXCESSIVE(%s()@%s:%d): found domain in address (%d bytes): %.*s"
#define LOG_DEBUGX_TLS_CERTIFICATE      "EXCESSIVE(%s()@%s:%d): loaded TLS certificate from file: %s"
#define LOG_DEBUGX_TLS_PRIVATEKEY_SEPARATE      "EXCESSIVE(%s()@%s:%d): loaded TLS private key from separate file: %s"
#define LOG_DEBUGX_TLS_PRIVATEKEY_CERTIFICATE   "EXCESSIVE(%s()@%s:%d): loaded TLS private key from certificate file: %s"
#define LOG_DEBUGX_TLS_CIPHERLIST                "EXCESSIVE(%s()@%s:%d): set TLS/SSL ciphers to: %s"
#define LOG_DEBUGX_TLS_CERT_CHECK               "EXCESSIVE(%s()@%s:%d): verified TLS certificate and private key"
#define LOG_DEBUGX_ENVIRONMENT_FOUND    "EXCESSIVE(%s()@%s:%d): found environment variable %.*s: %s"
#define LOG_DEBUGX_PATH_DEFAULT         "EXCESSIVE(%s()@%s:%d): no PATH found in environment, using default PATH: %s"
#define LOG_DEBUGX_PATH_SEARCH          "EXCESSIVE(%s()@%s:%d): searching along PATH: %s"
#define LOG_DEBUGX_ADDRESS_FOUND_QUOTE_OPEN     "EXCESSIVE(%s()@%s:%d): found opening quote in address at position %d: %s"
#define LOG_DEBUGX_ADDRESS_FOUND_QUOTE_CLOSE    "EXCESSIVE(%s()@%s:%d): found closing quote in address at position %d: %s"
#define LOG_DEBUGX_ADDRESS_NO_QUOTE_CLOSE       "EXCESSIVE(%s()@%s:%d): no closing quote found in address, assuming no quoted-string and resuming at position %d: %s"
#define LOG_DEBUGX_ADDRESS_ILLEGAL_CHAR         "EXCESSIVE(%s()@%s:%d): removing illegal character in address at position %d: %s"
#define LOG_DEBUGX_ADDRESS_ILLEGAL_DOT_START    "EXCESSIVE(%s()@%s:%d): removing illegal dot at start of domain: %s"
#define LOG_DEBUGX_ADDRESS_ILLEGAL_DOT_END      "EXCESSIVE(%s()@%s:%d): removing illegal dot at end of domain: %s"
#define LOG_DEBUGX_ADDRESS_ILLEGAL_DOT          "EXCESSIVE(%s()@%s:%d): ignoring illegal dot at start or end of username: %s"
#define LOG_DEBUGX_ADDRESS_FOUND_BRACKET_OPEN   "EXCESSIVE(%s()@%s:%d): found opening bracket in domain at position %d: %s"
#define LOG_DEBUGX_ADDRESS_FOUND_BRACKET_CLOSE  "EXCESSIVE(%s()@%s:%d): found closing bracket in domain at position %d: %s"
#define LOG_DEBUGX_ADDRESS_NO_BRACKET_CLOSE     "EXCESSIVE(%s()@%s:%d): no closing bracket found in domain, assuming no domain-literal and resuming at position %d: %s"
#define LOG_DEBUGX_TLS_DELAY            "EXCESSIVE(%s()@%s:%d): TLS operation did not complete, already waited %d seconds"
#define LOG_DEBUGX_DNS_NUM_QUESTIONS_ANSWERS    "EXCESSIVE(%s()@%s:%d): DNS packet contains %d questions, %d answers"
#define LOG_DEBUGX_DNS_TRUNCATED                "EXCESSIVE(%s()@%s:%d): DNS packet ID %d/%d truncated flag is set"
#define LOG_DEBUGX_DNS_QUERY_TCP        "EXCESSIVE(%s()@%s:%d): sending %d byte query (ID %d/%d) for %s(%s) via TCP"
#define LOG_DEBUGX_DNS_CONNECT          "EXCESSIVE(%s()@%s:%d): connecting to DNS server %s:%d via TCP"
#define LOG_DEBUGX_DNS_COUNTS           "EXCESSIVE(%s()@%s:%d): DNS packet ID %d/%d contains %d questions, %d answers"
#define LOG_DEBUGX_DNS_RECEIVED_TCP     "EXCESSIVE(%s()@%s:%d): received %d bytes via TCP, %d bytes so far in this response, expecting %d total"
#define LOG_DEBUGX_DNS_EMPTY_DATA       "EXCESSIVE(%s()@%s:%d): DNS data contains 0 bytes, ignoring response"

#define ERROR_CONFIG_NO_COMMAND         "ERROR: Missing qmail-smtpd command"
#define ERROR_CONFIG_UNKNOWN_OPTION     "ERROR: Unknown or incomplete option: %s"
#define ERROR_CONFIG_UNKNOWN_OPTION_FILE        "ERROR: Unknown configuration file option in file %s on line %d: %s"
#define ERROR_CONFIG_BAD_VALUE          "ERROR: Bad or unparsable value for option %s: %s"
#define ERROR_CONFIG_BAD_INTEGER_RANGE  "ERROR: Illegal value for option %s: %s (must be between %d and %d)"
#define ERROR_CONFIG_BAD_NAME           "ERROR: Illegal value for option %s: %s (must be one of %s)"
#define ERROR_CONFIG_BAD_LENGTH         "ERROR: Value for option %s is %d characters, length limit is %d characters"
#define ERROR_CONFIG_ILLEGAL_OPTION_CMDLINE     "ERROR: Option not allowed on command line: %s"
#define ERROR_CONFIG_ILLEGAL_OPTION_FILE        "ERROR: Option not allowed in configuration file, found in file %s on line %d: %s"
#define ERROR_CONFIG_SYNTAX_OPTION_FILE         "ERROR: Bad syntax in configuration file %s on line %d: %.*s"

#define LOG_ACTION_LOG_IP                       -8
#define LOG_ACTION_LOG_RDNS                     -7
#define LOG_ACTION_TLS_PASSTHROUGH_START        -6
#define LOG_ACTION_AUTH_FAILURE                 -5
#define LOG_ACTION_AUTH_SUCCESS                 -4
#define LOG_ACTION_TLS_START                    -3
#define LOG_ACTION_TLS_END                      -2
#define LOG_ACTION_NONE                         -1
#define LOG_ACTION_REMOTE_FROM                  0
#define LOG_ACTION_CHILD_FROM                   1
#define LOG_ACTION_CHILD_FROM_DISCARDED         2
#define LOG_ACTION_FILTER_FROM                  3
#define LOG_ACTION_FILTER_TO                    4
#define LOG_ACTION_LOG_OUTPUT                   5
#define LOG_ACTION_CURRENT_CONFIG               6
#define LOG_ACTION_CURRENT_ENVIRONMENT          7
#define LOG_ACTION_PREFIX                       (char *[]){ "FROM REMOTE TO CHILD", "FROM CHILD TO REMOTE", "FROM CHILD, FILTERED", "FROM SPAMDYKE TO REMOTE", "FROM SPAMDYKE TO CHILD", "LOG OUTPUT", "CURRENT CONFIG", "CURRENT ENVIRONMENT" }
#define LOG_ACTION_PREFIX_NONE                  ""
#define LOG_ACTION_PREFIX_TLS_SPAMDYKE          " TLS"
#define LOG_ACTION_PREFIX_TLS_PASSTHROUGH       " TLS_PASSTHROUGH"
#define LOG_ACTION_PREFIX_AUTH                  " AUTH:"
#define LOG_MESSAGE_TLS_PASSTHROUGH_START       "TLS passthrough started"
#define LOG_MESSAGE_TLS_START                   "TLS negotiated and started"
#define LOG_MESSAGE_TLS_END                     "TLS ended and closed"
#define LOG_MESSAGE_AUTH_SUCCESS                "Authentication successful"
#define LOG_MESSAGE_AUTH_FAILURE                "Authentication failed: "
#define LOG_MESSAGE_REMOTE_IP                   "Remote IP = "
#define LOG_MESSAGE_RDNS_NAME                   "Remote rDNS = "

#define LOG_MESSAGE_DNS_SEPARATOR       ", "
#define LOG_MESSAGE_DNS_TYPE_A          "A"
#define LOG_MESSAGE_DNS_TYPE_CNAME      "CNAME"
#define LOG_MESSAGE_DNS_TYPE_MX         "MX"
#define LOG_MESSAGE_DNS_TYPE_NS         "NS"
#define LOG_MESSAGE_DNS_TYPE_PTR        "PTR"
#define LOG_MESSAGE_DNS_TYPE_SOA        "SOA"
#define LOG_MESSAGE_DNS_TYPE_TXT        "TXT"

#define CONFIG_TEST_OPTION_NAME_BINARY  "binary-check"

#define CONFIG_TEST_ENVIRONMENT_LOCAL_PORT      "TCPLOCALPORT=25"
#define CONFIG_TEST_ENVIRONMENT_REMOTE_IP               { ENVIRONMENT_REMOTE_IP_TCPSERVER ENVIRONMENT_DELIMITER_STRING LOCALHOST_IP, ENVIRONMENT_REMOTE_IP_OLD_INETD ENVIRONMENT_DELIMITER_STRING LOCALHOST_IP, NULL }
#define CONFIG_TEST_STRLEN_ENVIRONMENT_REMOTE_IP        { STRLEN(ENVIRONMENT_REMOTE_IP_TCPSERVER ENVIRONMENT_DELIMITER_STRING LOCALHOST_IP), STRLEN(ENVIRONMENT_REMOTE_IP_OLD_INETD ENVIRONMENT_DELIMITER_STRING LOCALHOST_IP), -1 }
#define CONFIG_TEST_ENVIRONMENT_REMOTE_NAME     "TCPREMOTEHOST=localhost"

#define CONFIG_TEST_BAD_CONFIG_DIR_EXEC "ERROR(%s): Impossible test condition (DIR_EXEC). Please report this error to the author."
#define CONFIG_TEST_BAD_CONFIG_CMD_READ "ERROR(%s): Impossible test condition (CMD_READ). Please report this error to the author."
#define CONFIG_TEST_BAD_CONFIG_CMD_WRITE        "ERROR(%s): Impossible test condition (CMD_WRITE). Please report this error to the author."
#define CONFIG_TEST_BAD_CONFIG_CMD_READ_WRITE   "ERROR(%s): Impossible test condition (CMD_READ_WRITE). Please report this error to the author."

#define CONFIG_TEST_START               "Testing configuration..."
#define CONFIG_TEST_SUCCESS             "SUCCESS: Tests complete. No errors detected."
#define CONFIG_TEST_ERROR               "ERROR: Tests complete. Errors detected."
#define CONFIG_TEST_MISSING             "ERROR: config-test support was not included when spamdyke was compiled."

#define CONFIG_TEST_SUCCESS_UID         "SUCCESS: Running tests as user %s(%d), group %s(%d)."
#define CONFIG_TEST_WARNING_UID         "WARNING: Running tests as user %s(%d), group %s(%d). Is this the same user and group the mail server uses?"
#define CONFIG_TEST_ERROR_UID           "WARNING: Running tests as superuser %s(%d), group %s(%d). These test results may not be valid if the mail server runs as another user."

#define CONFIG_TEST_SUCCESS_SETUID      "SUCCESS: spamdyke binary (%s) is not owned by root and/or is not marked setuid."
#define CONFIG_TEST_ERROR_SETUID        "ERROR: spamdyke binary (%s) is owned by root and marked setuid. This is not necessary or recommended; it could be a security hole if exploitable bugs exist in spamdyke."
#define CONFIG_TEST_ERROR_SETUID_STAT   "ERROR: Unable to stat() spamdyke binary (%s) to scan permissions: %s"
#define CONFIG_TEST_ERROR_SETUID_SEARCH "ERROR: Unable to find spamdyke binary (%s) in the current path."
#define CONFIG_TEST_ERROR_SETUID_FILENAME       "ERROR: Name of current binary is unknown. This condition should be impossible."

#define CONFIG_TEST_START_FILE_READ     "INFO(%s): Testing file read: %s"
#define CONFIG_TEST_SUCCESS_FILE_READ   "SUCCESS(%s): Opened for reading: %s"
#define CONFIG_TEST_ERROR_FILE_READ     "ERROR(%s): Failed to open for reading: %s: %s"
#define CONFIG_TEST_START_FILE_WRITE    "INFO(%s): Testing file write: %s"
#define CONFIG_TEST_SUCCESS_FILE_WRITE  "SUCCESS(%s): Opened for writing: %s"
#define CONFIG_TEST_ERROR_FILE_WRITE    "ERROR(%s): Failed to open for writing: %s: %s"
#define CONFIG_TEST_START_FILE_READ_WRITE       "INFO(%s): Testing file reading and writing: %s"
#define CONFIG_TEST_SUCCESS_FILE_READ_WRITE     "SUCCESS(%s): Opened for reading and writing: %s"
#define CONFIG_TEST_ERROR_FILE_READ_WRITE       "ERROR(%s): Failed to open for reading and writing: %s: %s"
#define CONFIG_TEST_START_EXECUTE       "INFO(%s): Testing executable: %s"
#define CONFIG_TEST_SUCCESS_EXECUTE     "SUCCESS(%s): File is executable: %s"
#define CONFIG_TEST_ERROR_EXECUTE       "ERROR(%s): File is not executable: %s: %s"
#define CONFIG_TEST_START_DIR_READ      "INFO(%s): Testing directory tree and all files for reading: %s"
#define CONFIG_TEST_SUCCESS_DIR_READ    "SUCCESS(%s): Directory tree and all files are readable: %s"
#define CONFIG_TEST_ERROR_DIR_READ      "ERROR(%s): Portions of directory tree and/or some files are not readable: %s: %s"
#define CONFIG_TEST_START_DIR_WRITE     "INFO(%s): Testing directory for writing: %s"
#define CONFIG_TEST_SUCCESS_DIR_WRITE   "SUCCESS(%s): Created and deleted file in directory: %s"
#define CONFIG_TEST_ERROR_DIR_WRITE     "ERROR(%s): Failed to create file in directory: %s: %s"
#define CONFIG_TEST_ERROR_DIR_WRITE_DELETE      "ERROR(%s): Failed to delete test file in directory: %s: %s"
#define CONFIG_TEST_ERROR_FILE_OVERLENGTH       "ERROR(%s): File is too long; all content after line %d will be ignored: %s"

#define CONFIG_TEST_FILE_LINE_RECOMMENDATION    100
#define CONFIG_TEST_ERROR_FILE_OVERRECOMMENDATION       "WARNING(%s): File length is inefficient; consider using a directory structure instead: %s"

#define CONFIG_TEST_START_TLS           "INFO(%s): Testing TLS by initializing SSL/TLS library with certificate and key"
#define CONFIG_TEST_START_TLS_PRIVATEKEY        "INFO(%s): Testing TLS private key file for reading: %s"
#define CONFIG_TEST_SUCCESS_TLS         "SUCCESS(%s): Certificate and key loaded; SSL/TLS library successfully initialized"
#define CONFIG_TEST_ERROR_TLS_CERT_DISABLED             "ERROR(%s): TLS support is not compiled into this executable but a TLS certificate file was given anyway: %s"
#define CONFIG_TEST_ERROR_TLS_PRIVATEKEY_DISABLED       "ERROR(%s): TLS support is not compiled into this executable but a TLS private key file was given anyway: %s"
#define CONFIG_TEST_ERROR_TLS_PASSWORD_DISABLED         "ERROR(%s): TLS support is not compiled into this executable but a TLS private key password was given anyway."

#define CONFIG_TEST_SMTPAUTH_START      "INFO(%s): Examining authentication command: %s"
#define CONFIG_TEST_SMTPAUTH_OWNER_WARN "WARNING(%s): Authentication command is not owned by root. Some require being setuid root to read system passwords: %s: owned by %s(%d)"
#define CONFIG_TEST_SMTPAUTH_SETUID_WARN        "WARNING(%s): Authentication command is owned by root but not setuid. Some require being setuid root to read system passwords: %s"
#define CONFIG_TEST_SMTPAUTH_RUN_PLAIN  "INFO(%s): Running authentication command with unencrypted input: %s"
#define CONFIG_TEST_SUCCESS_SMTPAUTH_PLAIN      "SUCCESS(%s): Authentication succeeded with unencrypted input: %s"
#define CONFIG_TEST_FAILURE_SMTPAUTH_PLAIN      "ERROR(%s): Authentication failed with unencrypted input: %s"
#define CONFIG_TEST_SMTPAUTH_RUN_ENCRYPTED      "INFO(%s): Running authentication command with encrypted input: %s"
#define CONFIG_TEST_SUCCESS_SMTPAUTH_ENCRYPTED  "SUCCESS(%s): Authentication succeeded with encrypted input: %s"
#define CONFIG_TEST_FAILURE_SMTPAUTH_ENCRYPTED  "ERROR(%s): Authentication failed with encrypted input: %s"
#define CONFIG_TEST_SMTPAUTH_SUGGEST_ENCRYPTED  "INFO: One or more authentication commands support encrypted input; change the value of \"smtp-auth-level\" to \"%s\" or \"%s\" instead of \"%s\""
#define CONFIG_TEST_SMTPAUTH_SUGGEST_PLAIN      "INFO: No authentication commands support encrypted input; change the value of \"smtp-auth-level\" to \"%s\" or \"%s\" instead of \"%s\""
#define CONFIG_TEST_SMTPAUTH_UNUSED     "WARNING: None of the \"smtp-auth-command\" options will be used; \"smtp-auth-level\" is too low. Use a value of at least \"%s\""

#define CONFIG_TEST_TYPE_IFIFO          "FIFO (i.e. a named pipe)"
#define CONFIG_TEST_TYPE_IFCHR          "character device (e.g. a serial port)"
#define CONFIG_TEST_TYPE_IFDIR          "directory"
#define CONFIG_TEST_TYPE_IFBLK          "block device (e.g. a hard disk)"
#define CONFIG_TEST_TYPE_IFREG          "regular file"
#define CONFIG_TEST_TYPE_IFLNK          "symbolic link"
#define CONFIG_TEST_TYPE_IFSOCK         "socket"
#define CONFIG_TEST_TYPE_IFWHT          "whiteout"
#define CONFIG_TEST_TYPE_UNKNOWN        "filesystem entry of unknown type"

#define CONFIG_TEST_MSG_REGULAR_FILE    "File is not a regular file, it is a(n) %s"
#define CONFIG_TEST_MSG_OWNER_NO_EXEC   "Owner permissions apply but owner executable bit is not set"
#define CONFIG_TEST_MSG_PGROUP_NO_EXEC  "Group permissions apply but group executable bit is not set"
#define CONFIG_TEST_MSG_SGROUP_NO_EXEC  "Group permissions apply (through a secondary group) but group executable bit is not set"
#define CONFIG_TEST_MSG_OTHER_NO_EXEC   "\"Other\" permissions apply but \"other\" executable bit is not set"

#define CONFIG_TEST_START_GRAYLIST      "INFO(%s): Testing graylist directory: %s"
#define CONFIG_TEST_SUCCESS_GRAYLIST    "SUCCESS(%s): Graylist directory tests succeeded: %s"
#define CONFIG_TEST_ERROR_GRAYLIST_NONE_OPTIONS "ERROR(%s): The \"graylist-level\" option is \"none\" but other graylist options were given. They will all be ignored."
#define CONFIG_TEST_ERROR_GRAYLIST_TOP_EMPTY    "ERROR(%s): No domain directories found in graylist directory: %s"
#define CONFIG_TEST_ERROR_GRAYLIST_TOP_DIR      "ERROR(%s): Unable to read graylist directory %s: %s"
#define CONFIG_TEST_ERROR_GRAYLIST_TOP_OTHER    "ERROR(%s): Found %s in graylist directory where only domain directories should be: %s"
#define CONFIG_TEST_ERROR_GRAYLIST_TOP_ORPHAN   "ERROR(%s): Found domain directory for a domain that is not in the list of local domains; the domain directory will not be used: %s"
#define CONFIG_TEST_ERROR_GRAYLIST_DOMAIN_DIR   "ERROR(%s): Unable to read graylist domain directory %s: %s"
#define CONFIG_TEST_ERROR_GRAYLIST_DOMAIN_OTHER "ERROR(%s): Found %s in graylist domain directory where only user directories should be: %s"
#define CONFIG_TEST_ERROR_GRAYLIST_USER_DIR     "ERROR(%s): Unable to read graylist user directory %s: %s"
#define CONFIG_TEST_ERROR_GRAYLIST_USER_OTHER   "ERROR(%s): Found %s in graylist user directory where only user directories should be: %s"
#define CONFIG_TEST_ERROR_GRAYLIST_SENDER_DIR   "ERROR(%s): Unable to read graylist sender directory %s: %s"
#define CONFIG_TEST_ERROR_GRAYLIST_SENDER_OTHER "ERROR(%s): Found %s in graylist sender directory where only sender files should be: %s"
#define CONFIG_TEST_ERROR_GRAYLIST_DOMAIN_MISSING       "INFO(%s): Local domain has no domain directory; no graylisting will take place for the domain: %s"
#define CONFIG_TEST_ERROR_GRAYLIST_DOMAIN_CREATE        "INFO(%s): Local domain has no domain directory; spamdyke will create the directory when needed: %s"

#define CONFIG_TEST_START_CONFIGURATION_DIR     "INFO(%s): Testing configuration directory: %s"
#define CONFIG_TEST_SUCCESS_CONFIGURATION_DIR   "SUCCESS(%s): Configuration directory tests succeeded: %s"
#define CONFIG_TEST_ERROR_CONFIGURATION_DIR_TOP_OTHER   "ERROR(%s): Path to configuration directory is not a directory, it is a %s: %s"
#define CONFIG_TEST_ERROR_CONFIGURATION_DIR_DUPLICATE_DIR       "ERROR(%s): Found multiple configuration subdirectories named \"%s\" in the same path. This directory structure is invalid and will be ignored. Full path: %s"
#define CONFIG_TEST_ERROR_CONFIGURATION_DIR_DUPLICATE_USERNAME  "ERROR(%s): Found multiple configuration subdirectories named \"%s\" in the same path that are decendents of a \"%s\" directory. This directory structure is invalid and will be ignored. Full path: %s"
#define CONFIG_TEST_ERROR_CONFIGURATION_DIR_MISPLACED_USERNAME  "ERROR(%s): Found a configuration subdirectory named \"%s\" that is not a decendent of a \"%s\" directory or a \"%s\" directory. This directory structure is invalid and will be ignored. Full path: %s"
#define CONFIG_TEST_ERROR_CONFIGURATION_DIR_IP_BAD_OCTET        "ERROR(%s): Found a directory named \"%s\" that is not an integer between 0 and 255 but is a decendent of a \"%s\" directory. This directory structure is invalid and will be ignored. Full path: %s"
#define CONFIG_TEST_ERROR_CONFIGURATION_DIR_IP_TOO_DEEP "ERROR(%s): Found too many decendents of a \"%s\" directory (IP addresses can only have 4 octets). This directory structure is invalid and will be ignored. Full path: %s"
#define CONFIG_TEST_ERROR_CONFIGURATION_DIR_USERNAME_TOO_DEEP   "ERROR(%s): Found too many decendents of a \"%s\" directory (email addresses can only have 1 username). This directory structure is invalid and will be ignored. Full path: %s"
#define CONFIG_TEST_ERROR_CONFIGURATION_DIR_MISSING_DATA        "ERROR(%s): Found a \"%s\" directory as an immediate decendent of a \"%s\" directory. This directory structure is invalid and will be ignored. Full path: %s"
#define CONFIG_TEST_ERROR_CONFIGURATION_DIR_BAD_FILENAME        "ERROR(%s): Found a file named \"%s\", which should only be used for directory names. This file name is invalid and will be ignored. Full path: %s"
#define CONFIG_TEST_ERROR_CONFIGURATION_DIR_OPENDIR     "ERROR(%s): Unable to read configuration directory %s: %s"
#define CONFIG_TEST_ERROR_CONFIGURATION_DIR_BAD_TYPE    "ERROR(%s): Found an unexpected object (%s) in a directory structure that should only contain files and directories. This object is invalid and will be ignored. Full path: %s"

#define CONFIG_TEST_START_RDNS_DIR      "INFO(%s): Testing rDNS directory: %s"
#define CONFIG_TEST_SUCCESS_RDNS_DIR    "SUCCESS(%s): rDNS directory tests succeeded: %s"
#define CONFIG_TEST_ERROR_RDNS_LETTER_DIR       "ERROR(%s): rDNS directory name is longer than one character: %s"
#define CONFIG_TEST_ERROR_RDNS_LETTER_MISMATCH  "ERROR(%s): rDNS directory name does not start with the same single character as its parent: %s"
#define CONFIG_TEST_ERROR_RDNS_FQDN_MISMATCH    "ERROR(%s): rDNS entry does not match domain name %s: %s"
#define CONFIG_TEST_ERROR_RDNS_NO_FILES "ERROR(%s): rDNS directory contains no files: %s"
#define CONFIG_TEST_ERROR_RDNS_NO_FOLDERS       "ERROR(%s): rDNS directory contains no subdirectories: %s"
#define CONFIG_TEST_ERROR_RDNS_OPENDIR  "ERROR(%s): Unable to read rDNS directory: %s"
#define CONFIG_TEST_ERROR_RDNS_NON_DIR  "ERROR(%s): Found %s in rDNS directory where only directories should be: %s"
#define CONFIG_TEST_ERROR_RDNS_NON_FILE "ERROR(%s): Found %s in rDNS directory where only files should be: %s"

#define CONFIG_TEST_PATCH_SUCCESS_CONTINUATION  "\r\n250-"
#define CONFIG_TEST_PATCH_SUCCESS_END           "\r\n250 "
#define CONFIG_TEST_PATCH_TLS                   "starttls\r\n"
#define CONFIG_TEST_PATCH_SMTP_AUTH             "auth "
#define CONFIG_TEST_PATCH_EXPECT_GREETING       "220 "
#define CONFIG_TEST_PATCH_EXPECT_EHLO           "\r\n250 "
#define CONFIG_TEST_PATCH_SEND_EHLO             "EHLO localhost\r\n"
#define CONFIG_TEST_PATCH_SEND_QUIT             "QUIT\r\n"
#define CONFIG_TEST_PATCH_SCRIPT                { \
                                                  { ES_TYPE_EXPECT, CONFIG_TEST_PATCH_EXPECT_GREETING, STRLEN(CONFIG_TEST_PATCH_EXPECT_GREETING) }, \
                                                  { ES_TYPE_SEND, CONFIG_TEST_PATCH_SEND_EHLO, STRLEN(CONFIG_TEST_PATCH_SEND_EHLO) }, \
                                                  { ES_TYPE_EXPECT, CONFIG_TEST_PATCH_EXPECT_EHLO, STRLEN(CONFIG_TEST_PATCH_EXPECT_EHLO) }, \
                                                  { ES_TYPE_SEND, CONFIG_TEST_PATCH_SEND_QUIT, STRLEN(CONFIG_TEST_PATCH_SEND_QUIT) }, \
                                                  { ES_TYPE_NONE, NULL, 0 } \
                                                }
#define CONFIG_TEST_PATCH_RUN                   "INFO: Running command to test capabilities: %s"
#define CONFIG_TEST_ERROR_PATCH_NO_OUTPUT       "ERROR: Command returned no output: %s"
#define CONFIG_TEST_SUCCESS_PATCH_TLS           "SUCCESS: %s appears to offer TLS support but spamdyke will intercept and decrypt the TLS traffic so all of its filters can operate."
#define CONFIG_TEST_SUCCESS_PATCH_TLS_NO_TLS    "ERROR: %s appears to offer TLS support. The \"tls-type\" and \"tls-certificate-file\" options are being used but TLS support is not compiled into spamdyke. Unless it is recompiled with TLS support, the following spamdyke features will not function during TLS deliveries: graylisting, sender whitelisting, sender blacklisting, sender domain MX checking, DNS RHSBL checking for sender domains, recipient whitelisting, recipient blacklisting, limited number of recipients and full logging."
#define CONFIG_TEST_SUCCESS_PATCH_TLS_FLAG      "WARNING: %s appears to offer TLS support but spamdyke cannot use all of its filters unless it can intercept and decrypt the TLS traffic. Please use (or change) the \"tls-type\" and \"tls-certificate-file\" options. Otherwise, the following spamdyke features will not function during TLS deliveries: graylisting, sender whitelisting, sender blacklisting, sender domain MX checking, DNS RHSBL checking for sender domains, recipient whitelisting, recipient blacklisting, limited number of recipients and full logging."
#define CONFIG_TEST_SUCCESS_PATCH_TLS_FLAG_NO_TLS       "WARNING: %s appears to offer TLS support but spamdyke was not compiled with TLS support. spamdyke cannot use all of its filters unless it can intercept and decrypt the TLS traffic. Please recompile spamdyke with TLS support and use (or change) the \"tls-type\" and \"tls-certificate-file\" options. Unless it is recompiled with TLS support, the following spamdyke features will not function during TLS deliveries: graylisting, sender whitelisting, sender blacklisting, sender domain MX checking, DNS RHSBL checking for sender domains, recipient whitelisting, recipient blacklisting, limited number of recipients and full logging."
#define CONFIG_TEST_ERROR_PATCH_TLS             "SUCCESS: %s does not appear to offer TLS support. spamdyke will offer, intercept and decrypt TLS traffic."
#define CONFIG_TEST_ERROR_PATCH_TLS_NO_TLS      "ERROR: %s does not appear to offer TLS support and spamdyke was not compiled with TLS support. The \"tls-type\" and \"tls-certificate-file\" options will be ignored. Please recompile spamdyke with TLS support."
#define CONFIG_TEST_ERROR_PATCH_TLS_FLAG        "WARNING: %s does not appear to offer TLS support. Please use (or change) the \"tls-type\" and \"tls-certificate-file\" options so spamdyke can offer, intercept or decrypt TLS traffic."
#define CONFIG_TEST_ERROR_PATCH_TLS_FLAG_NO_TLS "ERROR: %s does not appear to offer TLS support and spamdyke was not compiled with TLS support. The \"tls-type\" and \"tls-certificate-file\" options will be ignored. Please recompile spamdyke with TLS support."
#define CONFIG_TEST_SUCCESS_PATCH_SMTP_AUTH_NONE        "SUCCESS: %s appears to offer SMTP AUTH support but spamdyke will block authentication attempts."
#define CONFIG_TEST_SUCCESS_PATCH_SMTP_AUTH_NONE_FLAG   "ERROR: %s appears to offer SMTP AUTH support but spamdyke will block authentication attempts. The \"smtp-auth-command\" option was given but will be ignored."
#define CONFIG_TEST_SUCCESS_PATCH_SMTP_AUTH_OBSERVE     "SUCCESS: %s appears to offer SMTP AUTH support. spamdyke will observe any authentication and trust its response."
#define CONFIG_TEST_SUCCESS_PATCH_SMTP_AUTH_OBSERVE_FLAG        "ERROR: %s appears to offer SMTP AUTH support. spamdyke will observe any authentication and trust its response. The \"smtp-auth-command\" option was given but will be ignored."
#define CONFIG_TEST_SUCCESS_PATCH_SMTP_AUTH_DEMAND      "SUCCESS: %s appears to offer SMTP AUTH support. spamdyke will observe any authentication and trust its response. spamdyke will offer authentication if %s does not."
#define CONFIG_TEST_SUCCESS_PATCH_SMTP_AUTH_DEMAND_FLAG "ERROR: %s appears to offer SMTP AUTH support. spamdyke will observe any authentication and trust its response but spamdyke cannot process responses itself because one or more of the following options was not given: \"access-file\", \"local-domains-file\" or \"smtp-auth-command\""
#define CONFIG_TEST_SUCCESS_PATCH_SMTP_AUTH_ALWAYS      "SUCCESS: %s appears to offer SMTP AUTH support but spamdyke will offer and process all authentication itself."
#define CONFIG_TEST_SUCCESS_PATCH_SMTP_AUTH_ALWAYS_FLAG "ERROR: %s appears to offer SMTP AUTH support but spamdyke cannot offer and process authentication itself because one of the following options was not given: \"access-file\", \"local-domains-file\" or \"smtp-auth-command\""
#define CONFIG_TEST_SUCCESS_PATCH_SMTP_AUTH_FLAG        "WARNING: %s appears to offer SMTP AUTH support but the \"smtp-auth-command\", \"smtp-auth-command-encryption\" and/or \"access-file\" options are in use. This is not necessary and needlessly creates extra load on the server."
#define CONFIG_TEST_ERROR_PATCH_SMTP_AUTH_NONE  "SUCCESS: %s does not appear to offer SMTP AUTH support. spamdyke will block authentication attempts."
#define CONFIG_TEST_ERROR_PATCH_SMTP_AUTH_NONE_FLAG     "ERROR: %s does not appear to offer SMTP AUTH support. spamdyke will block authentication attempts. The \"smtp-auth-command\" option was given but will be ignored."
#define CONFIG_TEST_ERROR_PATCH_SMTP_AUTH_OBSERVE       "SUCCESS: %s does not appear to offer SMTP AUTH support. spamdyke will observe any authentication and trust its response (although that appears unlikely to happen)."
#define CONFIG_TEST_ERROR_PATCH_SMTP_AUTH_OBSERVE_FLAG  "ERROR: %s does not appear to offer SMTP AUTH support. spamdyke will observe any authentication and trust its response (although that appears unlikely to happen). The \"smtp-auth-command\" option was given but will be ignored."
#define CONFIG_TEST_ERROR_PATCH_SMTP_AUTH_DEMAND        "SUCCESS: %s does not appear to offer SMTP AUTH support. spamdyke will offer and process authentication."
#define CONFIG_TEST_ERROR_PATCH_SMTP_AUTH_DEMAND_FLAG   "ERROR: %s does not appear to offer SMTP AUTH support. spamdyke cannot offer and process authentication because one of the following options was not given: \"access-file\", \"local-domains-file\" or \"smtp-auth-command\""
#define CONFIG_TEST_ERROR_PATCH_SMTP_AUTH_ALWAYS        "SUCCESS: %s does not appear to offer SMTP AUTH support. spamdyke will offer and process all authentication itself."
#define CONFIG_TEST_ERROR_PATCH_SMTP_AUTH_ALWAYS_FLAG   "ERROR: %s does not appear to offer SMTP AUTH support but spamdyke cannot offer and process authentication itself because one of the following options was not given: \"access-file\", \"local-domains-file\" or \"smtp-auth-command\""

#define CONFIG_TEST_OPTION_ARRAY_LIMIT          25
#define CONFIG_TEST_ERROR_OPTION_ARRAY          "WARNING: %s is used %d times; to increase efficiency, consider moving those values to a file instead."

#define CONFIG_TEST_ERROR_RELAY_NO_RELAY_MISSING_LOCAL  "ERROR(%s): The \"relay-level\" option is \"block-all\" but no local domains were given with \"local-domains-entry\" or \"local-domains-file\". The \"relay-level\" option will be ignored."
#define CONFIG_TEST_ERROR_RELAY_NORMAL_MISSING_LOCAL    "ERROR(%s): The \"relay-level\" option is \"normal\" but no local domains were given with \"local-domains-entry\" or \"local-domains-file\". The \"relay-level\" option will be ignored."
#define CONFIG_TEST_ERROR_RELAY_NORMAL_MISSING_ACCESS   "ERROR(%s): The \"relay-level\" option is \"normal\" but no access files were given with \"access-file\". The \"relay-level\" option will be ignored."

#define CONFIG_TYPE_NONE                        -3
/* Used for options that trigger an action rather than setting a variable */
#define CONFIG_TYPE_ACTION_ONCE                 -2
#define CONFIG_TYPE_ACTION_MULTIPLE             -1
/* True/false option */
#define CONFIG_TYPE_BOOLEAN                     0
/* Numeric option */
#define CONFIG_TYPE_INTEGER                     1
/* Text values */
#define CONFIG_TYPE_STRING_SINGLETON            2
#define CONFIG_TYPE_STRING_ARRAY                3
/* A single filename, can only be set once */
#define CONFIG_TYPE_FILE_SINGLETON              4
/* A single filename that has an alternate directory option */
#define CONFIG_TYPE_FILE_NOT_DIR_SINGLETON      5
/* Multiple filenames, stored in an array */
#define CONFIG_TYPE_FILE_ARRAY                  6
/* Multiple filenames that have an alternate directory option */
#define CONFIG_TYPE_FILE_NOT_DIR_ARRAY          7
/* A single directory, can only be set once */
#define CONFIG_TYPE_DIR_SINGLETON               8
/* Multiple directories, stored in an array */
#define CONFIG_TYPE_DIR_ARRAY                   9
/* A single command path with arguments, can only be set once */
#define CONFIG_TYPE_COMMAND_SINGLETON           10
/* Multiple command paths with arguments, stored in an array */
#define CONFIG_TYPE_COMMAND_ARRAY               11
/*
 * A text value that is matched against an array of values and stored as an
 * integer
 */
#define CONFIG_TYPE_NAME_ONCE                   12
/*
 * Multiple text values that are matched against an array of values to find
 * an integer value, then bitwise-ORed together
 */
#define CONFIG_TYPE_NAME_MULTIPLE               13
/*
 * A single string value that usually has an alternate file or directory option,
 * can only be set once
 */
#define CONFIG_TYPE_OPTION_SINGLETON            14
/*
 * A string value that usually has an alternate file or directory, stored in an
 * array
 */
#define CONFIG_TYPE_OPTION_ARRAY                15

#define CONFIG_ACCESS_NONE                      0
#define CONFIG_ACCESS_READ_ONLY                 1
#define CONFIG_ACCESS_WRITE_ONLY                2
#define CONFIG_ACCESS_READ_WRITE                3
#define CONFIG_ACCESS_EXECUTE                   4

#define CONFIG_LOCATION_MASK_ERRORS_CRITICAL    0x0F
#define CONFIG_LOCATION_MASK_ERRORS_FORGIVEN    0xF0

#define CONFIG_LOCATION_MASK_BASE_OPTIONS       0x0F
#define CONFIG_LOCATION_MASK_COPY_OPTIONS       0xF0

#define CONFIG_LOCATION_CMDLINE                 0x01
#define CONFIG_LOCATION_GLOBAL_FILE             0x02
#define CONFIG_LOCATION_DIR                     0x10

#define CONFIG_SET_ACTION(CMD)                  ({ int action(struct filter_settings *current_settings, int current_return_value, char *input_value, struct previous_action *history) { CMD; return(current_return_value); } &action; })
#define CONFIG_ACTION(CMD)                      ({ int action(struct filter_settings *current_settings, int current_return_value) { CMD; return(current_return_value); } &action; })
#define CONFIG_ACCESSOR_INTEGER(MEMBER)         ({ int *access_integer(struct option_set *current_options) { return(&current_options->MEMBER); } &access_integer; })
#define CONFIG_ACCESSOR_STRING(MEMBER)          ({ char **access_string(struct option_set *current_options) { return(&current_options->MEMBER); } &access_string; })
#define CONFIG_ACCESSOR_STRING_ARRAY(MEMBER)    ({ char ***access_string_array(struct option_set *current_options) { return(&current_options->MEMBER); } &access_string_array; })

#define ES_TYPE_NONE                            0
#define ES_TYPE_SEND                            1
#define ES_TYPE_EXPECT                          2

struct nihdns_header
  {
  uint16_t id;
  uint16_t bitfields;
  uint16_t qdcount;
  uint16_t ancount;
  uint16_t nscount;
  uint16_t arcount;
  };

struct expect_send
  {
  int type;
  char *data;
  int strlen_data;
  };

struct filter_settings;

struct previous_action
  {
  char *data;
  int count;
  struct previous_action *prev;
  };

struct option_set
  {
  /* These member variables are not accessible from option_list in
   * prepare_settings() -- they must be initialized/cleared explicitly
   * in init_option_set() and free_option_set().
   */
  int prev_filter_action;
  int filter_action;
  int filter_action_locked;
  int filter_grace;

  struct rejection_data *rejection;
  struct rejection_data *transient_rejection;
  struct rejection_data rejection_buf;
  struct rejection_data transient_rejection_buf;
  char reject_message_buf[MAX_BUF + 1];
  char short_reject_message_buf[MAX_BUF + 1];
  char transient_reject_message_buf[MAX_BUF + 1];
  char transient_short_reject_message_buf[MAX_BUF + 1];

  int strlen_policy_location;

  struct sockaddr_in nihdns_primary_server_data[MAX_NIHDNS_SERVERS + 1];
  struct sockaddr_in nihdns_secondary_server_data[MAX_NIHDNS_SERVERS + 1];

  /*
   * All members of this struct must be accessable from option_list in
   * prepare_settings() so they can be free()d by looping through that
   * structure at the end of prepare_settings() and free_current_options().
   */
  char *rejection_text[sizeof(REJECTION_DATA) / sizeof(struct rejection_data)];
  char *run_user;
  char **config_file;
  int filter_level;

  char *local_server_name;
  char *local_server_name_file;
  char *local_server_name_command;

  char **graylist_dir;
  int graylist_min_secs;
  int graylist_max_secs;
  int max_rcpt_to;
  int log_target;
  int log_level;
  int relay_level;
  char *policy_location;
  char *log_dir;
  char **blacklist_sender;
  char **blacklist_sender_file;
  char **whitelist_sender;
  char **whitelist_sender_file;
  char **blacklist_recipient;
  char **blacklist_recipient_file;
  char **whitelist_recipient;
  char **whitelist_recipient_file;
  int configuration_dir_search;
  char **configuration_dir;

  char **access_list_file;
  char **local_domains;
  char **local_domains_file;
  char **blacklist_rdns_keyword;
  char **blacklist_rdns_keyword_file;
  char **whitelist_rdns_keyword;
  char **whitelist_rdns_keyword_file;
  char **blacklist_rdns;
  char **blacklist_rdns_file;
  char **blacklist_rdns_dir;
  char **blacklist_ip;
  char **blacklist_ip_file;
  char **whitelist_rdns;
  char **whitelist_rdns_file;
  char **whitelist_rdns_dir;
  char **whitelist_ip;
  char **whitelist_ip_file;
  char **dnsrwl_fqdn;
  char **dnsrwl_fqdn_file;
  char **dnsrbl_fqdn;
  char **dnsrbl_fqdn_file;
  char **rhswl_fqdn;
  char **rhswl_fqdn_file;
  char **rhsbl_fqdn;
  char **rhsbl_fqdn_file;
  char **graylist_exception_ip;
  char **graylist_exception_ip_file;
  char **graylist_exception_rdns;
  char **graylist_exception_rdns_file;
  char **graylist_exception_rdns_dir;
  char **smtp_auth_command;
  int smtp_auth_level;

  int graylist_level;
  int check_ip_in_rdns_cc;
  int check_earlytalker;
  int check_rdns_exist;
  int check_rdns_resolve;
  int check_sender_mx;
  int check_identical_from_to;

  int timeout_connection;
  int timeout_command;

  int nihdns_level;
  int nihdns_tcp;
  int nihdns_spoof;
  char **nihdns_primary_server_list;
  char **nihdns_secondary_server_list;
  int nihdns_attempts_primary;
  int nihdns_attempts_total;
  int nihdns_timeout_total_secs;
  char **nihdns_resolv_conf;

  int tls_level;
  char *tls_certificate_file;
  char *tls_privatekey_file;
  char *tls_cipher_list;
  int strlen_tls_privatekey_password;
  char *tls_privatekey_password;
  char *tls_privatekey_password_file;

  char *test_smtp_auth_username;
  char *test_smtp_auth_password;
  };

struct integer_string
  {
  int *integers;
  char **strings;
  };

struct spamdyke_option
  {
  int value_type;
  int access_type;
  int location;
  struct option getopt_option;
  union
    {
    int integer_value;
    char *string_value;
    } default_value;
  union
    {
    int integer_value;
    char *string_value;
    } missing_value;
  union
    {
    int *(*get_integer)(struct option_set *);
    char **(*get_string)(struct option_set *);
    char ***(*get_string_array)(struct option_set *);
    } getter;
  union
    {
    int max_strlen;
    struct
      {
      int minimum;
      int maximum;
      } integer_range;
    struct integer_string string_list;
    } validity;
  int set_consequence;
  int set_grace;
  int (*test_function)(struct filter_settings *, struct spamdyke_option *);
  int (*additional_set_actions)(struct filter_settings *, int, char *, struct previous_action *);
  int (*additional_actions)(struct filter_settings *, int);
  char *help_argument;
  char *help_text;
  };

struct filter_settings
  {
  struct option_set base_options;
  struct option_set *current_options;

  struct spamdyke_option *option_list;
  int num_options;
  struct option *long_options;
  char short_options[MAX_BUF + 1];
  struct spamdyke_option **option_lookup;
  int max_short_code;

  /* original_environment must always be the envp value passed to main() */
  char **original_environment;
  /*
   * current_environment may contain values from original_environment but must
   * never contain static strings that cannot be free()d.
   */
  char **current_environment;

  char server_name[MAX_BUF + 1];
  int strlen_server_name;
  char *server_ip;
  char tmp_server_ip[MAX_BUF + 1];
  int strlen_server_ip;
  int ip_in_server_name;

  int allow_relay;
  char additional_domain_text[MAX_BUF + 1];
  int inside_data;
  int num_rcpt_to;
  int local_sender;
  int local_recipient;

  char sender_username[MAX_ADDRESS + 1];
  char sender_domain[MAX_ADDRESS + 1];
  char recipient_username[MAX_ADDRESS + 1];
  char recipient_domain[MAX_ADDRESS + 1];
  char configuration_path[MAX_PATH + 1];

  char **child_argv;

  int smtp_auth_state;
  int smtp_auth_type;
  int smtp_auth_origin;
  char smtp_auth_challenge[MAX_BUF + 1];
  char smtp_auth_response[MAX_BUF + 1];
  char smtp_auth_username[MAX_BUF + 1];

  time_t connection_start;
  time_t command_start;

  int tls_state;

#ifdef HAVE_LIBSSL

  SSL_CTX *tls_context;
  SSL *tls_session;

#endif /* HAVE_LIBSSL */

  };

#endif /* SPAMDYKE_H */
