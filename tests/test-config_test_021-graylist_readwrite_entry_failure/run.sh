# This test looks for an error message from the config-test when it finds a
# graylist entry it cannot read/write.

if [ "${UID}" == "0" ]
then
  mkdir -p ${TMPDIR}/${TEST_NUM}-graylist.d/example.com/foo/baz.com
  touch ${TMPDIR}/${TEST_NUM}-graylist.d/example.com/foo/baz.com/bar
  chmod 444 ${TMPDIR}/${TEST_NUM}-graylist.d/example.com/foo/baz.com/bar

  pushd ..
  su $4 ./subrun $1 $2 $3 $4 $5 test-${TEST_NUM}-*
  popd

  output=`grep "Failed to open for reading and writing: ${TMPDIR}/${TEST_NUM}-graylist.d/example.com/foo/baz.com/bar" ${TMPDIR}/${TEST_NUM}-output.txt`
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
