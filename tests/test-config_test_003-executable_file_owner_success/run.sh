# This test looks for a success message from the config-test when it finds an
# executable file owned by the user.

if [ "${UID}" == "0" ]
then
  touch ${TMPDIR}/${TEST_NUM}-hostname
  chmod 111 ${TMPDIR}/${TEST_NUM}-hostname

  pushd ..
  su $4 ./subrun $1 $2 $3 $4 $5 test-${TEST_NUM}-*
  popd

  output=`grep "File is executable: ${TMPDIR}/${TEST_NUM}-hostname" ${TMPDIR}/${TEST_NUM}-output.txt`
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
  echo "${SPAMDYKE_PATH} -ldebug --hostname-command ${TMPDIR}/${TEST_NUM}-hostname --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
  ${SPAMDYKE_PATH} -ldebug --hostname-command ${TMPDIR}/${TEST_NUM}-hostname --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1
fi
