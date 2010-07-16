# This test looks for a rejection because the incoming IP address' rDNS name
# contains the IP address and a banned keyword pattern

export TCPREMOTEIP=$TESTSD_IP_IN_RDNS_PATTERN_IP
echo ${TESTSD_IP_IN_RDNS_PATTERN} > ${TMPDIR}/${TEST_NUM}-rdns_keyword_pattern.txt

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} -k ${TMPDIR}/${TEST_NUM}-rdns_keyword_pattern.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} -k ${TMPDIR}/${TEST_NUM}-rdns_keyword_pattern.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "554 Refused. Your reverse DNS entry contains your IP address and a banned keyword." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
