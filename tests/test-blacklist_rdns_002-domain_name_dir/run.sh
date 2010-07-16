# This test looks for a rejection because the incoming rDNS name is blacklisted
# by base domain name.

export TCPREMOTEIP=${TESTSD_RDNS_IP}

FQDN=`${DNSPTR_PATH} ${TESTSD_RDNS_IP} | ${DOMAINSPLIT_PATH}`
mkdir -p ${TMPDIR}/${TEST_NUM}-blacklist.d/`${DOMAIN2PATH_PATH} -d ${FQDN}`
touch ${TMPDIR}/${TEST_NUM}-blacklist.d/`${DOMAIN2PATH_PATH} ${FQDN}`

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} -b ${TMPDIR}/${TEST_NUM}-blacklist.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} -b ${TMPDIR}/${TEST_NUM}-blacklist.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "554 Refused. Your domain name is blacklisted." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
