# This test looks for a rejection when the sender's domain has no MX record.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@foo.example.com

touch ${TMPDIR}/${TEST_NUM}-local_domains.txt

echo rejection-text-missing-sender-mx=Foo Bar Baz >> ${TMPDIR}/${TEST_NUM}-config.txt
echo reject-missing-sender-mx >> ${TMPDIR}/${TEST_NUM}-config.txt
echo local-domains-file=${TMPDIR}/${TEST_NUM}-local_domains.txt >> ${TMPDIR}/${TEST_NUM}-config.txt

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "421 Foo Bar Baz" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "421 Refused. The domain of your sender address has no mail exchanger (MX)." ${TMPDIR}/${TEST_NUM}-output.txt`
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
