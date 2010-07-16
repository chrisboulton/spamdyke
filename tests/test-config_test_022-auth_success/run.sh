# This test looks for a success message from the config-test when it finds an
# SMTP AUTH command executable, owned by root and setuid.

touch ${TMPDIR}/${TEST_NUM}-smtp-auth
chmod 4755 ${TMPDIR}/${TEST_NUM}-smtp-auth

echo "${SPAMDYKE_PATH} -ldebug --smtp-auth-command ${TMPDIR}/${TEST_NUM}-smtp-auth --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -ldebug --smtp-auth-command ${TMPDIR}/${TEST_NUM}-smtp-auth --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "File is executable: ${TMPDIR}/${TEST_NUM}-smtp-auth" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
