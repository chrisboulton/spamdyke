# This test looks for a rejection because the incoming rDNS name is on a
# RHSBL that uses TXT records.

export TCPREMOTEIP=11.22.33.44

echo "44.33.22.11.in-addr.arpa PTR NORMAL foo.example.com" > ${TMPDIR}/${TEST_NUM}-dns_config.txt
echo "example.com.txt.rhsbl TXT NORMAL Test RHSBL match" >> ${TMPDIR}/${TEST_NUM}-dns_config.txt

NAMESERVER_IP=127.0.0.1:`${DNSDUMMY_PATH} -t 30 -f ${TMPDIR}/${TEST_NUM}-dns_config.txt`

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@foo.bar

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} --log-target stderr -lverbose --dns-server-ip ${NAMESERVER_IP} -X nonexistent1.rhsbl -X nonexistent2.rhsbl -X nonexistent3.rhsbl -X txt.rhsbl -X nonexistent4.rhsbl ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} --log-target stderr -lverbose --dns-server-ip ${NAMESERVER_IP} -X nonexistent1.rhsbl -X nonexistent2.rhsbl -X nonexistent3.rhsbl -X txt.rhsbl -X nonexistent4.rhsbl ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "554 Test RHSBL match" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "FILTER_RHSBL_MATCH domain: foo.example.com rhsbl: txt.rhsbl" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
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
