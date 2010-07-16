# This test looks for a success message from the config-test when it finds a
# writable directory.

if [ "${UID}" == "0" ]
then
  mkdir ${TMPDIR}/${TEST_NUM}-logs
  chmod 333 ${TMPDIR}/${TEST_NUM}-logs

  pushd ..
  su $4 ./subrun $1 $2 $3 $4 $5 test-${TEST_NUM}-*
  popd

  output=`grep "Created and deleted file in directory: ${TMPDIR}/${TEST_NUM}-logs" ${TMPDIR}/${TEST_NUM}-output.txt`
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
else
  echo "${SPAMDYKE_PATH} -ldebug --full-log-dir ${TMPDIR}/${TEST_NUM}-logs --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
  ${SPAMDYKE_PATH} -ldebug --full-log-dir ${TMPDIR}/${TEST_NUM}-logs --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1
fi
