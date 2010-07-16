# This test unsets a value from a string array with multiple values.  The value
# should be removed.

export TCPREMOTEIP=11.22.33.44
export PRIMARY_NAMESERVER_IP_1=127.0.0.125
export PRIMARY_NAMESERVER_IP_2=127.0.0.126
export PRIMARY_NAMESERVER_IP_3=127.0.0.127

echo log-level=excessive >> ${TMPDIR}/${TEST_NUM}-config.txt
echo log-target=stderr >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-server-ip-primary=${PRIMARY_NAMESERVER_IP_1} >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-server-ip-primary=${PRIMARY_NAMESERVER_IP_2} >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-server-ip-primary=${PRIMARY_NAMESERVER_IP_3} >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-server-ip-primary=!${PRIMARY_NAMESERVER_IP_1} >> ${TMPDIR}/${TEST_NUM}-config.txt

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${PRIMARY_NAMESERVER_IP_1}:53 (attempt 1)" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ -z "${output}" ]
then
  primary_count_2=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${PRIMARY_NAMESERVER_IP_2}:53" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
  primary_count_3=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${PRIMARY_NAMESERVER_IP_3}:53" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
  total_count=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
  if [ "$[${primary_count_2}+${primary_count_3}]" == "${total_count}" ]
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
