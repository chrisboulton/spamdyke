# This test looks for a rejection when the remote server is not allowed to
# relay.

export TCPREMOTEIP=10.255.128.64

echo "10.64.128.255:allow,RELAYCLIENT=\"\"" > ${TMPDIR}/${TEST_NUM}-access.txt
echo ":allow" >> ${TMPDIR}/${TEST_NUM}-access.txt

TO_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com
FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

touch ${TMPDIR}/${TEST_NUM}-local_domains.txt

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com
echo rejection-text-relaying-denied=Foo Bar Baz >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/${TO_ADDRESS}/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} --access-file ${TMPDIR}/${TEST_NUM}-access.txt --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} --access-file ${TMPDIR}/${TEST_NUM}-access.txt --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "554 Foo Bar Baz" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "554 Refused. Sending to remote addresses (relaying) is not allowed." ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ -z "${output}" ] 
  then
    outcome="success"
  else
    echo Filter failure - tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
