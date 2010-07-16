# This test looks for an error if the short options in configuration.c are not
# unique.

echo "${SPAMDYKE_PATH} -v > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -v > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep -E "ERROR: short option . is used by at least two options" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
