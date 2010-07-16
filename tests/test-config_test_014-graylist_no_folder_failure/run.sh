# This test looks for an error message from the config-test when it finds a
# missing graylist directory.

echo "${SPAMDYKE_PATH} -ldebug -g ${TMPDIR}/${TEST_NUM}-graylist.d --graylist-level always --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -ldebug -g ${TMPDIR}/${TEST_NUM}-graylist.d --graylist-level always --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "Unable to read graylist directory ${TMPDIR}/${TEST_NUM}-graylist.d" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "ERROR: Tests complete. Errors detected." ${TMPDIR}/${TEST_NUM}-output.txt`
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
