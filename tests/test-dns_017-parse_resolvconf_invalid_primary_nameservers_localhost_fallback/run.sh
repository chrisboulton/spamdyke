# This test alters the /etc/resolv.conf file to set the nameserver to an
# invalid value, then starts spamdyke with an invalid DNS server IP address.
# spamdyke should ignore both invalid values and use localhost instead.

export TCPREMOTEIP=11.22.33.44

if [ -f /etc/resolv.conf ]
then
  cp /etc/resolv.conf ${TMPDIR}/${TEST_NUM}-resolv.conf.bak
fi
cat resolv_conf.txt | sed -e "s/NAMESERVER_IP/bar/g" > /etc/resolv.conf

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-timeout-secs 10 --dns-server-ip-primary foo ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-timeout-secs 10 --dns-server-ip-primary foo ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: invalid/unparsable nameserver found: bar" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server 127.0.0.1:53 (attempt 1)" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    output=`grep "ERROR: invalid/unparsable nameserver found: foo" ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ ! -z "${output}" ]
    then
      outcome="success"
    else
      echo CONTENTS OF /etc/resolv.conf:
      cat /etc/resolv.conf
      echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
      cat ${TMPDIR}/${TEST_NUM}-output.txt

      outcome="failure"
    fi
  else
    echo CONTENTS OF /etc/resolv.conf:
    cat /etc/resolv.conf
    echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
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
