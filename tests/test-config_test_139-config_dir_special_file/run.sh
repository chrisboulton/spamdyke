# This test runs spamdyke's config-test when the path given to config-dir
# contains a special file.

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d
mkfifo ${TMPDIR}/${TEST_NUM}-config.d/foo

echo "${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR(config-dir): Found an unexpected object (FIFO (i.e. a named pipe)) in a directory structure that should only contain files and directories. This object is invalid and will be ignored. Full path: ${TMPDIR}/${TEST_NUM}-config.d/foo" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
