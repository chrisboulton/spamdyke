# This test runs spamdyke with -h to check the help message.

echo "${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} -h > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} -h > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "USAGE: spamdyke" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
