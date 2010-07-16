# This test alters the /etc/resolv.conf file to set the nameserver to a known
# value and starts spamdyke to see if it parsed the file and queried the
# nameserver.  A total timeout value is given, which spamdyke should parse and
# honor.

export TCPREMOTEIP=11.22.33.44
export NAMESERVER_IP=127.0.0.1:52
export TIMEOUT=18

if [ -f /etc/resolv.conf ]
then
  cp /etc/resolv.conf ${TMPDIR}/${TEST_NUM}-resolv.conf.bak
fi
cat resolv_conf.txt | sed -e "s/NAMESERVER_IP/${NAMESERVER_IP}/g" -e "s/TIMEOUT/${TIMEOUT}/g" > /etc/resolv.conf

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "time ${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
start_time=`date +%s`
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-timeout-secs ${TIMEOUT} ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1
end_time=`date +%s`

overtime=true
if [ "$[${end_time}-${start_time}]" = "${TIMEOUT}" ]
then
  overtime=false
elif [ "$[${end_time}-${start_time}]" = "$[${TIMEOUT}+1]" ]
then
  overtime=false
fi

if [ "${overtime}" == "false" ]
then
  output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_IP} (attempt 3)" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    outcome="success"
  else
    echo Total time was $[${end_time}-${start_time}], should have been ${TIMEOUT}
    echo CONTENTS OF /etc/resolv.conf:
    cat /etc/resolv.conf
    echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo Total time was $[${end_time}-${start_time}], should have been ${TIMEOUT}
  echo CONTENTS OF /etc/resolv.conf:
  cat /etc/resolv.conf
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi

if [ -f ${TMPDIR}/${TEST_NUM}-resolv.conf.bak ]
then
  cp ${TMPDIR}/${TEST_NUM}-resolv.conf.bak /etc/resolv.conf
fi
