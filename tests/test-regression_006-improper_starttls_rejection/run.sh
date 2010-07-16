# This test elicits a rejection response to MAIL FROM, then gives STARTTLS,
# even though it's at the wrong place.  spamdyke should respond with a 421
# and the rejection message anyway.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@foo.example.com

touch ${TMPDIR}/${TEST_NUM}-local_domains.txt

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --reject-missing-sender-mx -d ${TMPDIR}/${TEST_NUM}-local_domains.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --reject-missing-sender-mx -d ${TMPDIR}/${TEST_NUM}-local_domains.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "421 Refused. The domain of your sender address has no mail exchanger (MX)." ${TMPDIR}/${TEST_NUM}-output.txt | awk '{ print $1 }'`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
