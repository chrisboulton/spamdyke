# This test runs spamdyke with -v to check the version message.

echo "${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} -v > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} -v > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "Use -h for an option summary or see README.html for complete option details." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "ERROR:" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ -z "${output}" ]
  then
    output=`grep "USAGE" ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ -z "${output}" ]
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
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
