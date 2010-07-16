# This test looks for a rejection because the sender email address is
# blacklisted.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

echo ${FROM_ADDRESS} > ${TMPDIR}/${TEST_NUM}-sender_blacklist.txt

echo "sender-blacklist-file=${TMPDIR}/${TEST_NUM}-sender_blacklist.txt" > ${TMPDIR}/${TEST_NUM}-config.txt

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "554 Refused. Your sender address has been blacklisted." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
