# This test looks for a message from the config-test when it finds the child
# process supports SMTP AUTH and the smtp-auth-command option is given to
# spamdyke.

child_cmd=`echo ${QMAIL_CMDLINE} | awk '{ print $1 }'`

echo "${SPAMDYKE_PATH} -lverbose --config-test --smtp-auth-command \"${AUTH_CMDLINE}\" ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -lverbose --config-test --smtp-auth-command "${AUTH_CMDLINE}" ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: ${child_cmd} appears to offer SMTP AUTH support. spamdyke will observe any authentication and trust its response but spamdyke cannot process responses itself because one or more of the following options was not given: \"access-file\", \"local-domains-file\" or \"smtp-auth-command\"" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
