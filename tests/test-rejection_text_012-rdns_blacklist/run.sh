# This test looks for a rejection because the incoming rDNS name is blacklisted
# by FQDN in a file.

export TCPREMOTEIP=${TESTSD_RDNS_IP}

${DNSPTR_PATH} ${TESTSD_RDNS_IP} > ${TMPDIR}/${TEST_NUM}-blacklist_rdns.txt

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} --rejection-text-rdns-blacklist \"Foo Bar Baz\" --rdns-blacklist-file ${TMPDIR}/${TEST_NUM}-blacklist_rdns.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} --rejection-text-rdns-blacklist "Foo Bar Baz" --rdns-blacklist-file ${TMPDIR}/${TEST_NUM}-blacklist_rdns.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "554 Foo Bar Baz" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "554 Refused. Your domain name is blacklisted." ${TMPDIR}/${TEST_NUM}-output.txt`
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
