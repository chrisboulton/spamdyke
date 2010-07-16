# This test looks for a rejection because the recipient email address is
# blacklisted by domain name.  The given recipient address contains the
# domain name embedded in the username, which could confuse the address
# parser.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

echo "@example.com" > ${TMPDIR}/${TEST_NUM}-recipient_blacklist.txt

cat input.txt | sed -e "s/TARGET_EMAIL/asdf.example.comuser@example.com/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} -S ${TMPDIR}/${TEST_NUM}-recipient_blacklist.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} -S ${TMPDIR}/${TEST_NUM}-recipient_blacklist.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "554 Refused. Mail is not being accepted at this address." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
