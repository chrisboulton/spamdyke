# This test looks for a success message from the config-test when it finds a
# graylist directories with domain and user directories

if [ "${UID}" == "0" ]
then
  mkdir -p ${TMPDIR}/${TEST_NUM}-graylist.d/example.com/foo/baz.com
  touch ${TMPDIR}/${TEST_NUM}-graylist.d/example.com/foo/baz.com/bar
  chown $4 `find ${TMPDIR}/${TEST_NUM}-graylist.d`

  pushd ..
  su $4 ./subrun $1 $2 $3 $4 $5 test-${TEST_NUM}-*
  popd

  output=`grep "Graylist directory tests succeeded: ${TMPDIR}/${TEST_NUM}-graylist.d" ${TMPDIR}/${TEST_NUM}-output.txt`
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
  echo "${SPAMDYKE_PATH} -ldebug -g ${TMPDIR}/${TEST_NUM}-graylist.d --graylist-level always --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
  ${SPAMDYKE_PATH} -ldebug -g ${TMPDIR}/${TEST_NUM}-graylist.d --graylist-level always --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1
fi
