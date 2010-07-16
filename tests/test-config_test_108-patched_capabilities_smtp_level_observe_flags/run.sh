# This test looks for a message from the config-test when it finds the child
# process supports SMTP AUTH, smtp-auth-level is "observe" and
# smtp-auth-command is given.

child_cmd=`echo ${QMAIL_CMDLINE} | awk '{ print $1 }'`

echo "${SPAMDYKE_PATH} -lverbose --config-test --smtp-auth-level observe --smtp-auth-command \"${AUTH_CMDLINE}\" ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -lverbose --config-test --smtp-auth-level observe --smtp-auth-command "${AUTH_CMDLINE}" ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "WARNING: None of the \"smtp-auth-command\" options will be used; \"smtp-auth-level\" is too low. Use a value of at least \"ondemand\"" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "ERROR: ${child_cmd} appears to offer SMTP AUTH support. spamdyke will observe any authentication and trust its response. The \"smtp-auth-command\" option was given but will be ignored." ${TMPDIR}/${TEST_NUM}-output.txt`
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
