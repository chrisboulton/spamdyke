# This test sets dns-spoof to accept-same-port and checks to make sure no spoofed
# packets were accepted.  FIXME: This test should be rewritten to find a way to
# send spoofed packets from another IP with the same port number.

export TCPREMOTEIP=11.22.33.44

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

echo "44.33.22.11.in-addr.arpa PTR SPOOF foo.bar" > ${TMPDIR}/${TEST_NUM}-dns_config.txt

NAMESERVER_IP=127.0.0.1:`${DNSDUMMY_PATH} -t 120 -f ${TMPDIR}/${TEST_NUM}-dns_config.txt`

cat input.txt | sed -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" -e "s/TARGET_EMAIL/$1/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 120 -r 221 -- ${SPAMDYKE_PATH} --reject-empty-rdns --dns-server-ip ${NAMESERVER_IP} --log-target stderr --dns-spoof accept-same-port ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 120 -r 221 -- ${SPAMDYKE_PATH} --reject-empty-rdns --dns-server-ip ${NAMESERVER_IP} --log-target stderr --dns-spoof accept-same-port ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "Refused. You have no reverse DNS entry." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
