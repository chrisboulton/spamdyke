# This test looks for a success message from the config-test when it finds the
# child process does not support SMTP AUTH and the SMTP AUTH option have been
# given.

touch ${TMPDIR}/${TEST_NUM}-access.txt
touch ${TMPDIR}/${TEST_NUM}-local_domains.txt

child_cmd=`echo ${QMAIL_CMDLINE} | awk '{ print $1 }'`

echo "${SPAMDYKE_PATH} -ldebug --smtp-auth-level ondemand --smtp-auth-command \"${AUTH_CMDLINE}\" --access-file ${TMPDIR}/${TEST_NUM}-access.txt --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -ldebug --smtp-auth-level ondemand --smtp-auth-command "${AUTH_CMDLINE}" --access-file ${TMPDIR}/${TEST_NUM}-access.txt --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "SUCCESS: ${child_cmd} does not appear to offer SMTP AUTH support. spamdyke will offer and process authentication." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
