# This test runs spamdyke with an integer value below its minimum and checks the
# error message.

echo "${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --dns-max-retries-total -1 > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --dns-max-retries-total -1 > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: Illegal value for option dns-max-retries-total: -1 (must be between 1 and 2147483647)" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
