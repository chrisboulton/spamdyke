# This test looks for a startup error because the configuration file can't be
# read after the user has been changed.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

echo reject-empty-rdns=yes >> ${TMPDIR}/${TEST_NUM}-config.txt
echo run-as-user=$4 >> ${TMPDIR}/${TEST_NUM}-config.txt
echo ip-whitelist-file=${TMPDIR}/${TEST_NUM}-whitelist_ip.txt >> ${TMPDIR}/${TEST_NUM}-config.txt
echo log-target=stderr >> ${TMPDIR}/${TEST_NUM}-config.txt

echo 0.0.0.0 > ${TMPDIR}/${TEST_NUM}-whitelist_ip.txt
chmod 600 ${TMPDIR}/${TEST_NUM}-whitelist_ip.txt

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: Option not allowed in configuration file, found in file ${TMPDIR}/${TEST_NUM}-config.txt on line 2: run-as-user" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
