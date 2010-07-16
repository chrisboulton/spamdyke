# This test looks for a crash when a DNS RBL returns a long record and excessive
# logging is enabled.

export TCPREMOTEIP=88.229.90.167

echo "167.90.229.88.txt.dnsrbl TXT NORMAL Latest spam received via gollum.manitu.net at Wed, 02 Jan 2008 11:58:35 +0100, see http://www.dnsbl.manitu.net/lookup.php?value=88.229.90.167" > ${TMPDIR}/${TEST_NUM}-dns_config.txt

NAMESERVER_IP=127.0.0.1:`${DNSDUMMY_PATH} -t 30 -f ${TMPDIR}/${TEST_NUM}-dns_config.txt`

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} -lexcessive --dns-server-ip ${NAMESERVER_IP} -x txt.dnsrbl ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} -lexcessive --dns-server-ip ${NAMESERVER_IP} -x txt.dnsrbl ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "554 Latest spam received via gollum.manitu.net" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
