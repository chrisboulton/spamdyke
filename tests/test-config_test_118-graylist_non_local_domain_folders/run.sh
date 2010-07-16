# This test looks for a message from the config-test when it finds a graylist
# domain directory for a domain that is not local.

mkdir ${TMPDIR}/${TEST_NUM}-graylist.d/foo.com

echo example.com > ${TMPDIR}/${TEST_NUM}-local_domains.txt

echo "${SPAMDYKE_PATH} -ldebug -g ${TMPDIR}/${TEST_NUM}-graylist.d --graylist-level always-create-dir --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -ldebug -g ${TMPDIR}/${TEST_NUM}-graylist.d --graylist-level always-create-dir --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR(graylist-level): Found domain directory for a domain that is not in the list of local domains; the domain directory will not be used: foo.com" ${TMPDIR}/${TEST_NUM}-output.txt`
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
