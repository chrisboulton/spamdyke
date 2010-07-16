# This test looks for a success message from the config-test when it tests the
# SMTP AUTH command and successfully authenticates with both encrypted and
# non-encrypted data.

echo "${SPAMDYKE_PATH} -ldebug --smtp-auth-level ondemand --smtp-auth-command \"${AUTH_CMDLINE}\" --config-test-smtpauth-username $2 --config-test-smtpauth-password $3 --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -ldebug --smtp-auth-level ondemand --smtp-auth-command "${AUTH_CMDLINE}" --config-test-smtpauth-username $2 --config-test-smtpauth-password $3 --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "SUCCESS(smtp-auth-level): Authentication succeeded with unencrypted input: " ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "SUCCESS(smtp-auth-level): Authentication succeeded with encrypted input: " ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    output=`grep "INFO: One or more authentication commands support encrypted input; change the value of \"smtp-auth-level\" to \"ondemand-encrypted\" or \"always-encrypted\" instead of \"ondemand\"" ${TMPDIR}/${TEST_NUM}-output.txt`
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
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
