# This test looks for a success message because the incoming rDNS address is on
# a RHSBL but the records are chained with CNAMEs and circular.

export TCPREMOTEIP=11.22.33.44

echo "44.33.22.11.in-addr.arpa PTR foo.example.com" > ${TMPDIR}/${TEST_NUM}-dns_config.txt
echo "example.com.circular.rhsbl CNAME 1.example.com.circular.rhsbl" >> ${TMPDIR}/${TEST_NUM}-dns_config.txt
echo "1.example.com.circular.rhsbl CNAME example.com.circular.rhsbl" >> ${TMPDIR}/${TEST_NUM}-dns_config.txt

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --dns-server-ip ${NAMESERVER_IP} -X circular.rhsbl ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --dns-server-ip ${NAMESERVER_IP} -X circular.rhsbl ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
