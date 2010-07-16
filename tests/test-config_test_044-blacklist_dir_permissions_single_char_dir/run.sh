# This test looks for an error message from the config-test when it finds a
# single character directory with permissions it can't read.

if [ "${UID}" == "0" ]
then
  mkdir -p ${TMPDIR}/${TEST_NUM}-rdns_blacklist.d/com/e/example
  touch ${TMPDIR}/${TEST_NUM}-rdns_blacklist.d/com/e/example/example.com

  chmod 000 ${TMPDIR}/${TEST_NUM}-rdns_blacklist.d/com/e

  pushd ..
  su $4 ./subrun $1 $2 $3 $4 $5 test-${TEST_NUM}-*
  popd

  output=`grep "ERROR(rdns-blacklist-dir): Unable to read rDNS directory" ${TMPDIR}/${TEST_NUM}-output.txt`
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
else
  echo "${SPAMDYKE_PATH} -ldebug -b ${TMPDIR}/${TEST_NUM}-rdns_blacklist.d --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
  ${SPAMDYKE_PATH} -ldebug -b ${TMPDIR}/${TEST_NUM}-rdns_blacklist.d --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1
fi
