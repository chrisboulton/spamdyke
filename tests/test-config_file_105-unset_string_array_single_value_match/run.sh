# This test unsets a value from a string array with only one value.  The value
# should be removed.

export TCPREMOTEIP=11.22.33.44
export PRIMARY_NAMESERVER_IP=127.0.0.127
export SECONDARY_NAMESERVER_IP=127.0.0.128

echo log-level=excessive >> ${TMPDIR}/${TEST_NUM}-config.txt
echo log-target=stderr >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-server-ip-primary=${PRIMARY_NAMESERVER_IP} >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-server-ip=${SECONDARY_NAMESERVER_IP} >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-server-ip-primary=!${PRIMARY_NAMESERVER_IP} >> ${TMPDIR}/${TEST_NUM}-config.txt

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${SECONDARY_NAMESERVER_IP}:53 (attempt 1)" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  secondary_count=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${SECONDARY_NAMESERVER_IP}:53" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
  total_count=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
  if [ "${secondary_count}" == "${total_count}" ]
  then
    outcome="success"
  else
    echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
