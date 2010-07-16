# This test checks the DNS query behavior when level is "none".
# No queries should be performed at all.

export TCPREMOTEIP=11.22.33.44
export NAMESERVER_PRIMARY_IP_1=127.0.0.1:52
export NAMESERVER_PRIMARY_IP_2=127.0.0.1:50
export NAMESERVER_SECONDARY_IP_1=127.0.0.1:51
export NAMESERVER_SECONDARY_IP_2=127.0.0.1:49

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-timeout-secs 10 --dns-server-ip-primary ${NAMESERVER_PRIMARY_IP_1} --dns-server-ip-primary ${NAMESERVER_PRIMARY_IP_2} --dns-server-ip ${NAMESERVER_SECONDARY_IP_1} --dns-server-ip ${NAMESERVER_SECONDARY_IP_2} --dns-level none ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-timeout-secs 10 --dns-server-ip-primary ${NAMESERVER_PRIMARY_IP_1} --dns-server-ip-primary ${NAMESERVER_PRIMARY_IP_2} --dns-server-ip ${NAMESERVER_SECONDARY_IP_1} --dns-server-ip ${NAMESERVER_SECONDARY_IP_2} --dns-level none ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
