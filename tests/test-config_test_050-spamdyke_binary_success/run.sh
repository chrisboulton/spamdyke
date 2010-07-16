# This test looks for a success message from the config-test when it finds an
# spamdyke binary that is not setuid root.

cp ${SPAMDYKE_PATH} ${TMPDIR}/${TEST_NUM}-spamdyke
chmod 755 ${TMPDIR}/${TEST_NUM}-spamdyke

echo "${TMPDIR}/${TEST_NUM}-spamdyke -ldebug --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${TMPDIR}/${TEST_NUM}-spamdyke -ldebug --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "SUCCESS: spamdyke binary (${TMPDIR}/${TEST_NUM}-spamdyke) is not owned by root and/or is not marked setuid." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
