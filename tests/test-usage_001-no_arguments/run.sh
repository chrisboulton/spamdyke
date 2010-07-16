# This test runs spamdyke with no arguments to check the error message.

echo "${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: Missing qmail-smtpd command" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
