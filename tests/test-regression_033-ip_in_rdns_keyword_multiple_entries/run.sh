# This test looks for a rejection because the incoming IP address' rDNS name
# contains the IP address and a banned keyword when a list of banned keywords
# are read from a file and some are longer than others.

export TCPREMOTEIP=$TESTSD_IP_IN_RDNS_KEYWORD_IP
echo x${TESTSD_IP_IN_RDNS_KEYWORD}x > ${TMPDIR}/${TEST_NUM}-rdns_keywords.txt
echo ${TESTSD_IP_IN_RDNS_KEYWORD} > ${TMPDIR}/${TEST_NUM}-rdns_keywords.txt

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} -k ${TMPDIR}/${TEST_NUM}-rdns_keywords.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} -k ${TMPDIR}/${TEST_NUM}-rdns_keywords.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "554 Refused. Your reverse DNS entry contains your IP address and a banned keyword." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
