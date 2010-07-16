# This test looks for a message from the config-test when it finds one or more
# graylist options but graylist-level is not given.

echo example.com > ${TMPDIR}/${TEST_NUM}-local_domains.txt

echo "${SPAMDYKE_PATH} -ldebug --graylist-level none -g ${TMPDIR}/${TEST_NUM}-graylist.d --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -ldebug --graylist-level none -g ${TMPDIR}/${TEST_NUM}-graylist.d --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR(graylist-level): The "graylist-level" option is "none" but other graylist options were given. They will all be ignored." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ -z "${output}" ]
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
