# This test runs spamdyke's config-test when the path given to config-dir
# contains an _at_ directory that is not a decendent of a _sender_ or _recipient_
# directory.

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_at_/one

echo "${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR(config-dir): Found a configuration subdirectory named \"_at_\" that is not a decendent of a \"_recipient_\" directory or a \"_sender_\" directory. This directory structure is invalid and will be ignored. Full path: ${TMPDIR}/${TEST_NUM}-config.d/_at_" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
