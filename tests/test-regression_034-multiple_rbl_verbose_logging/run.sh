# This test looks for a rejection because the incoming IP address is on a
# DNS RBL that uses TXT records.  When the logging is set to verbose and
# multiple DNS RBLs are used, the correct RBL should be reported, not the
# first one.

export TCPREMOTEIP=11.22.33.44

echo "44.33.22.11.txt.dnsrbl TXT NORMAL Test DNSRBL match" > ${TMPDIR}/${TEST_NUM}-dns_config.txt

NAMESERVER_IP=127.0.0.1:`${DNSDUMMY_PATH} -t 30 -f ${TMPDIR}/${TEST_NUM}-dns_config.txt`

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} --log-target stderr -lverbose --dns-server-ip ${NAMESERVER_IP} -x txt.nonexistant1.dnsrbl -x txt.nonexistant2.dnsrbl -x txt.nonexistant3.dnsrbl -x txt.nonexistant4.dnsrbl -x txt.nonexistant5.dnsrbl -x txt.nonexistant6.dnsrbl -x txt.nonexistant7.dnsrbl -x txt.nonexistant8.dnsrbl -x txt.nonexistant9.dnsrbl -x txt.nonexistant10.dnsrbl -x txt.dnsrbl ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} --log-target stderr -lverbose --dns-server-ip ${NAMESERVER_IP} -x txt.nonexistant1.dnsrbl -x txt.nonexistant2.dnsrbl -x txt.nonexistant3.dnsrbl -x txt.nonexistant4.dnsrbl -x txt.nonexistant5.dnsrbl -x txt.nonexistant6.dnsrbl -x txt.nonexistant7.dnsrbl -x txt.nonexistant8.dnsrbl -x txt.nonexistant9.dnsrbl -x txt.nonexistant10.dnsrbl -x txt.dnsrbl ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "554 Test DNSRBL match" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "FILTER_RBL_MATCH ip: 11.22.33.44 rbl: txt.dnsrbl" ${TMPDIR}/${TEST_NUM}-output.txt`
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
