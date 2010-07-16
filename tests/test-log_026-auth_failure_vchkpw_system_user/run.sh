# This test checks for the correct log message after authenticating with AUTH
# LOGIN using the "exitvalue" command to simulate an unsuccessful login due to
# a failed authentication attempt using the system password file.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com
AUTH_USERNAME=`${SMTPAUTH_LOGIN_PATH} ${TEST_NUM}-username ${TEST_NUM}-password | tail -3 | head -1 | awk '{ print $2 }'`
AUTH_PASSWORD=`${SMTPAUTH_LOGIN_PATH} ${TEST_NUM}-username ${TEST_NUM}-password | tail -1 | awk '{ print $2 }'`

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" -e "s/AUTH_USERNAME/${AUTH_USERNAME}/g" -e "s/AUTH_PASSWORD/${AUTH_PASSWORD}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -r -ldebug --log-target stderr --smtp-auth-level always --smtp-auth-command \"${EXITVALUE_PATH} 23\" ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -r -ldebug --log-target stderr --smtp-auth-level always --smtp-auth-command "${EXITVALUE_PATH} 23" ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: authentication error 23 (vchkpw uses this to indicate a failed authentication attempt using the system password file): ${TEST_NUM}-username" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Logging failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
