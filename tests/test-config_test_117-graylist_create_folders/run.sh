# This test looks for a message from the config-test when it finds an
# empty graylist directory but graylist-level is always-create-dir.

mkdir ${TMPDIR}/${TEST_NUM}-graylist.d

echo example.com > ${TMPDIR}/${TEST_NUM}-local_domains.txt

echo "${SPAMDYKE_PATH} -ldebug -g ${TMPDIR}/${TEST_NUM}-graylist.d --graylist-level always-create-dir --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -ldebug -g ${TMPDIR}/${TEST_NUM}-graylist.d --graylist-level always-create-dir --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "INFO(graylist-level): Local domain has no domain directory; spamdyke will create the directory when needed: example.com" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "SUCCESS: Tests complete. No errors detected." ${TMPDIR}/${TEST_NUM}-output.txt`
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
