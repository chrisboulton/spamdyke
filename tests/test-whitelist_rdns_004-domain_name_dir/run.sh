# This test looks for a rejection because the incoming rDNS name is whitelist_rdns.txt
# by base domain name.

export TCPREMOTEIP=${TESTSD_UNRESOLVABLE_RDNS_IP}

INCOMING_DOMAIN=`${DNSPTR_PATH} ${TESTSD_UNRESOLVABLE_RDNS_IP} | ${DOMAINSPLIT_PATH}`
mkdir -p ${TMPDIR}/${TEST_NUM}-whitelist_rdns.d/`${DOMAIN2PATH_PATH} -d ${INCOMING_DOMAIN}`
touch ${TMPDIR}/${TEST_NUM}-whitelist_rdns.d/`${DOMAIN2PATH_PATH} ${INCOMING_DOMAIN}`

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -R --rdns-whitelist-dir ${TMPDIR}/${TEST_NUM}-whitelist_rdns.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -R --rdns-whitelist-dir ${TMPDIR}/${TEST_NUM}-whitelist_rdns.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
