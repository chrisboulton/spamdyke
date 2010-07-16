# This test looks for failure messages from the config-test when it tests the
# SMTP AUTH command and fails to authenticate with both encrypted and
# non-encrypted data.

echo "${SPAMDYKE_PATH} -ldebug --smtp-auth-command \"${AUTH_CMDLINE}\" --config-test-smtpauth-username foo.$2 --config-test-smtpauth-password password --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -ldebug --smtp-auth-command "${AUTH_CMDLINE}" --config-test-smtpauth-username foo.$2 --config-test-smtpauth-password password --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR(smtp-auth-level): Authentication failed with unencrypted input: " ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "ERROR(smtp-auth-level): Authentication failed with encrypted input: " ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    outcome="success"
  else
    echo Failure - tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
