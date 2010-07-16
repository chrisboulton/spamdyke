# This test looks for a rejection because the incoming IP address' rDNS name
# contains the IP address and a country code.

export TCPREMOTEIP=$TESTSD_IP_IN_CC_RDNS

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com
echo "# Comment line" > ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "reject-ip-in-cc-rdns  =    yes  " >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "# Comment line" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "# Comment line" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "554 Refused. Your reverse DNS entry contains your IP address and a country code." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
