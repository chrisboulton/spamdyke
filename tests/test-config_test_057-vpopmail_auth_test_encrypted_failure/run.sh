# This test looks for failure messages from the config-test when it tests the
# SMTP AUTH command and authenticates with encrypted and fails to authenticate
# with non-encrypted data.

if [ -x /bin/checkpassword ]
then
  echo "${SPAMDYKE_PATH} -ldebug --smtp-auth-level ondemand-encrypted --smtp-auth-command \"/bin/checkpassword ${TRUE_PATH}\" --config-test-smtpauth-username $4 --config-test-smtpauth-password $5 --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
  ${SPAMDYKE_PATH} -ldebug --smtp-auth-level ondemand-encrypted --smtp-auth-command "/bin/checkpassword ${TRUE_PATH}" --config-test-smtpauth-username $4 --config-test-smtpauth-password $5 --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

  output=`grep "SUCCESS(smtp-auth-level): Authentication succeeded with unencrypted input:" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    output=`grep "ERROR(smtp-auth-level): Authentication failed with encrypted input: " ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ ! -z "${output}" ]
    then
      output=`grep "INFO: No authentication commands support encrypted input; change the value of \"smtp-auth-level\" to \"ondemand\" or \"always\" instead of \"ondemand-encrypted\"" ${TMPDIR}/${TEST_NUM}-output.txt`
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
else
  echo /bin/checkpassword not found.  Test skipped.
  outcome="skipped"
fi
