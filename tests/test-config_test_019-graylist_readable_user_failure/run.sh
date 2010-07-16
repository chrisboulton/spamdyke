# This test looks for an error message from the config-test when it finds an
# unreadable user directory.

if [ "${UID}" == "0" ]
then
  mkdir -p ${TMPDIR}/${TEST_NUM}-graylist.d/example.com/foo
  chmod 000 ${TMPDIR}/${TEST_NUM}-graylist.d/example.com/foo

  pushd ..
  su $4 ./subrun $1 $2 $3 $4 $5 test-${TEST_NUM}-*
  popd

  output=`grep "Unable to read graylist user directory ${TMPDIR}/${TEST_NUM}-graylist.d/example.com/foo" ${TMPDIR}/${TEST_NUM}-output.txt`
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
  echo "${SPAMDYKE_PATH} -ldebug -g ${TMPDIR}/${TEST_NUM}-graylist.d --graylist-level always --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
  ${SPAMDYKE_PATH} -ldebug -g ${TMPDIR}/${TEST_NUM}-graylist.d --graylist-level always --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1
fi
