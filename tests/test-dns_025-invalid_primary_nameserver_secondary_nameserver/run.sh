# This test passes a secondary nameserver IP and an invalid primary nameserver
# IP to spamdyke and checks to see if the primary IP was ignored.

export TCPREMOTEIP=11.22.33.44
export NAMESERVER_IP_PRIMARY=foo
export NAMESERVER_IP_SECONDARY=127.0.0.1:52

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-timeout-secs 10 --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY} --dns-server-ip ${NAMESERVER_IP_SECONDARY} ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-timeout-secs 10 --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY} --dns-server-ip ${NAMESERVER_IP_SECONDARY} ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_IP_SECONDARY} (attempt 1)" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_IP_PRIMARY}" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ -z "${output}" ]
  then
    output=`grep "ERROR: invalid/unparsable nameserver found: foo" ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ ! -z "${output}" ]
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
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
