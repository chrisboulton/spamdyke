# This test looks for an error message from the config-test when it finds a
# blacklist directory that contains no subdirectories.

mkdir -p ${TMPDIR}/${TEST_NUM}-rdns_blacklist.d

echo "${SPAMDYKE_PATH} -ldebug -b ${TMPDIR}/${TEST_NUM}-rdns_blacklist.d --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -ldebug -b ${TMPDIR}/${TEST_NUM}-rdns_blacklist.d --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR(rdns-blacklist-dir): rDNS directory contains no subdirectories" ${TMPDIR}/${TEST_NUM}-output.txt`
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
