# This test tries to exploit a bug: spamdyke ignores all input after a DATA
# command, even if the DATA command was rejected by qmail.  This lets a remote
# server bypass all recipient filters if it knows to send the SMTP commands
# in a specific sequence.

export TCPREMOTEIP=${TESTSD_RDNS_IP}

echo ":allow" > ${TMPDIR}/${TEST_NUM}-access.txt
echo "example.com" > ${TMPDIR}/${TEST_NUM}-local_domains.txt

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --access-file ${TMPDIR}/${TEST_NUM}-access.txt --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --access-file ${TMPDIR}/${TEST_NUM}-access.txt --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
