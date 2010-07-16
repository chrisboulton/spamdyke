# This test passes a new resolv.conf to spamdyke and checks to see if it
# was used.

export TCPREMOTEIP=11.22.33.44
export NAMESERVER_IP=127.0.0.128

echo "nameserver ${NAMESERVER_IP}" > ${TMPDIR}/${TEST_NUM}-resolv.conf

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-resolv-conf ${TMPDIR}/${TEST_NUM}-resolv.conf ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-resolv-conf ${TMPDIR}/${TEST_NUM}-resolv.conf ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_IP}:53 (attempt 1)" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
