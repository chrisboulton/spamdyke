# This test runs spamdyke with a "named option" option that has an
# illegal/unknown argument.

echo "${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --log-target asdf > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --log-target asdf > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: Illegal value for option log-target: asdf (must be one of stderr, syslog)" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
