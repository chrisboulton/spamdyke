# This test runs spamdyke's config-test when the path given to config-dir
# contains a file named _recipient_.

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d
touch ${TMPDIR}/${TEST_NUM}-config.d/_recipient_

echo "${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR(config-dir): Found a file named \"_recipient_\", which should only be used for directory names. This file name is invalid and will be ignored. Full path: ${TMPDIR}/${TEST_NUM}-config.d/_recipient_" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
