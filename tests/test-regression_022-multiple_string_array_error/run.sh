# This test looks for a rejection because the incoming IP address doesn't have
# an rDNS name.  The ip-whitelist-file option is set multiple times, which
# should not return an internal error that prevents the empty rDNS test from
# running.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

touch ${TMPDIR}/${TEST_NUM}-ip_whitelist_1.txt
touch ${TMPDIR}/${TEST_NUM}-ip_whitelist_2.txt

echo "reject-empty-rdns=yes" >> ${TMPDIR}/${TEST_NUM}-config.txt
echo "ip-whitelist-file=${TMPDIR}/${TEST_NUM}-ip_whitelist_1.txt" >> ${TMPDIR}/${TEST_NUM}-config.txt
echo "config-dir=${TMPDIR}/${TEST_NUM}-config.d" >> ${TMPDIR}/${TEST_NUM}-config.txt

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com
echo "ip-whitelist-file=${TMPDIR}/${TEST_NUM}-ip_whitelist_2.txt" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "421 Refused. You have no reverse DNS entry." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
