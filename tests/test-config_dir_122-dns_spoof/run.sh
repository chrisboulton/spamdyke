# This test attempts to set dns-spoof in a
# config dir and checks to make sure it was rejected.

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

export TCPREMOTEIP=11.22.33.44

echo "44.33.22.11.in-addr.arpa PTR SPOOF foo.bar" > ${TMPDIR}/${TEST_NUM}-dns_config.txt

NAMESERVER_IP=127.0.0.1:`${DNSDUMMY_PATH} -t 30 -f ${TMPDIR}/${TEST_NUM}-dns_config.txt`

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com
echo dns-spoof=reject >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example

cat input.txt | sed -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" -e "s/TARGET_EMAIL/$1/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --dns-server-ip ${NAMESERVER_IP} --reject-empty-rdns --config-dir ${TMPDIR}/${TEST_NUM}-config.d --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --dns-server-ip ${NAMESERVER_IP} --reject-empty-rdns --config-dir ${TMPDIR}/${TEST_NUM}-config.d --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: Option not allowed in configuration file, found in file ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example on line 1: dns-spoof" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "Refused. You have no reverse DNS entry." ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ -z "${output}" ]
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
