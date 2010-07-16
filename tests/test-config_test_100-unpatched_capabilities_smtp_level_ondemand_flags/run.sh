# This test looks for a message from the config-test when it finds the child
# process does not support SMTP AUTH, smtp-auth-level is "ondemand" and
# smtp-auth-command, access-file and local-domains-file are given.

child_cmd=`echo ${QMAIL_CMDLINE} | awk '{ print $1 }'`

touch ${TMPDIR}/${TEST_NUM}-local_domains.txt
touch ${TMPDIR}/${TEST_NUM}-access.txt

echo "${SPAMDYKE_PATH} -lverbose --config-test --smtp-auth-level ondemand --smtp-auth-command \"${AUTH_CMDLINE}\" --access-file ${TMPDIR}/${TEST_NUM}-access.txt --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -lverbose --config-test --smtp-auth-level ondemand --smtp-auth-command "${AUTH_CMDLINE}" --access-file ${TMPDIR}/${TEST_NUM}-access.txt --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "SUCCESS: ${child_cmd} does not appear to offer SMTP AUTH support. spamdyke will offer and process authentication." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
