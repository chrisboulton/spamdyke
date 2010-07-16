# This test looks for a failure message from the config-test when the child
# process is unresponsive and won't exit.

echo "${SPAMDYKE_PATH} -ldebug --log-target stderr --config-test ${SLEEP_PATH} 300 > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -ldebug --log-target stderr --config-test ${SLEEP_PATH} 300 > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: command aborted abnormally: " ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
