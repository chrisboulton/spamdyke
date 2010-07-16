# This test looks for a failure message from the config-test when it finds an
# spamdyke binary that is setuid root.

cp ${SPAMDYKE_PATH} ${TMPDIR}/${TEST_NUM}-spamdyke
chmod 4755 ${TMPDIR}/${TEST_NUM}-spamdyke

echo "${TMPDIR}/${TEST_NUM}-spamdyke -ldebug --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${TMPDIR}/${TEST_NUM}-spamdyke -ldebug --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: spamdyke binary (${TMPDIR}/${TEST_NUM}-spamdyke) is owned by root and marked setuid. This is not necessary or recommended; it could be a security hole if exploitable bugs exist in spamdyke." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
