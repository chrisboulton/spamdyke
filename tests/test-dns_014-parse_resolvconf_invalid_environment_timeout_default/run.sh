# This test alters the /etc/resolv.conf file to set the nameserver to a known
# value and starts spamdyke to see if it parsed the file and queried the
# nameserver.  An invalid timeout value is given in the environment variable
# RES_OPTIONS, which spamdyke should ignore.  No value is given in
# /etc/resolv.conf, so the default (30 seconds) should be used instead.

export TCPREMOTEIP=11.22.33.44
export NAMESERVER_IP=127.0.0.1:52
export TIMEOUT=30
export RES_OPTIONS="timeout:foo"

if [ -f /etc/resolv.conf ]
then
  cp /etc/resolv.conf ${TMPDIR}/${TEST_NUM}-resolv.conf.bak
fi
cat resolv_conf.txt | sed -e "s/NAMESERVER_IP/${NAMESERVER_IP}/g" > /etc/resolv.conf

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "time ${SENDRECV_PATH} -t 60 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
start_time=`date +%s`
${SENDRECV_PATH} -t 60 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-timeout-secs ${TIMEOUT} ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1
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
    output=`grep "ERROR: invalid/unparsable query timeout found in environment variable RES_OPTIONS: foo" ${TMPDIR}/${TEST_NUM}-output.txt`
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
