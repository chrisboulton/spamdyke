# This test runs spamdyke's config-test when the path given to config-dir
# contains a decendent of an _ip_ directory that is not an integer.

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_ip_/foo

echo "${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR(config-dir): Found a directory named \"foo\" that is not an integer between 0 and 255 but is a decendent of a \"_ip_\" directory. This directory structure is invalid and will be ignored. Full path: ${TMPDIR}/${TEST_NUM}-config.d/_ip_/foo" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
