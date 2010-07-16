# This test checks the DNS query behavior when level is "none".
# No queries should be performed at all.

export TCPREMOTEIP=11.22.33.44

echo log-level=excessive >> ${TMPDIR}/${TEST_NUM}-config.txt
echo log-target=stderr >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-timeout-secs=10 >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-server-ip-primary=127.0.0.1:52 >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-server-ip-primary=127.0.0.1:50 >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-server-ip=127.0.0.1:51 >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-server-ip=127.0.0.1:49 >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-level=none >> ${TMPDIR}/${TEST_NUM}-config.txt

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
