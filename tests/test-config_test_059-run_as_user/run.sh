# This test looks for a warning message from the config-test when it is run as
# an non-root user.

if [ "${UID}" == "0" ]
then
  pushd ..
  su $4 ./subrun $1 $2 $3 $4 $5 test-${TEST_NUM}-*
  popd

  output=`grep "WARNING: Running tests as user $4" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    outcome="success"
  else
    echo Failure - tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo "${SPAMDYKE_PATH} -ldebug --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
  ${SPAMDYKE_PATH} -ldebug --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1
fi
