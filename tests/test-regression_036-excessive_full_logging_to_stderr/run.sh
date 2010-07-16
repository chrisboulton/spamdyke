# This test starts spamdyke with a configuration file that asks for excessive
# logging and full logging, then looks for EXCESSIVE messages on stderr before
# the configuration file has been fully read.

export TCPREMOTEIP=0.0.0.0

mkdir -p ${TMPDIR}/${TEST_NUM}-log.d

echo "log-level=excessive" >> ${TMPDIR}/${TEST_NUM}-conf.txt
echo "full-log-dir=${TMPDIR}/${TEST_NUM}-log.d" >> ${TMPDIR}/${TEST_NUM}-conf.txt

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-conf.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-conf.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep -E "EXCESSIVE" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
