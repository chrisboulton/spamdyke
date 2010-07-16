# This test looks for an error message from the config-test when it finds an
# unreadable file.

if [ "${UID}" == "0" ]
then
  touch ${TMPDIR}/${TEST_NUM}-local-domains.txt
  chmod 000 ${TMPDIR}/${TEST_NUM}-local-domains.txt

  pushd ..
  su $4 ./subrun $1 $2 $3 $4 $5 test-${TEST_NUM}-*
  popd

  output=`grep "Failed to open for reading: ${TMPDIR}/${TEST_NUM}-local-domains.txt" ${TMPDIR}/${TEST_NUM}-output.txt`
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
  echo "${SPAMDYKE_PATH} -ldebug -d ${TMPDIR}/${TEST_NUM}-local-domains.txt --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
  ${SPAMDYKE_PATH} -ldebug -d ${TMPDIR}/${TEST_NUM}-local-domains.txt --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1
fi
