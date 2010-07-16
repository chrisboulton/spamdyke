# This test looks for a rejection because the incoming rDNS name is in a
# whitelist directory structure by FQDN.

export TCPREMOTEIP=${TESTSD_UNRESOLVABLE_RDNS_IP}

INCOMING_DOMAIN=`${DNSPTR_PATH} ${TESTSD_UNRESOLVABLE_RDNS_IP}`
mkdir -p ${TMPDIR}/${TEST_NUM}-whitelist_rdns.d/`${DOMAIN2PATH_PATH} -d ${INCOMING_DOMAIN}`
touch ${TMPDIR}/${TEST_NUM}-whitelist_rdns.d/`${DOMAIN2PATH_PATH} ${INCOMING_DOMAIN}`

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com
echo "reject-unresolvable-rdns=yes" > ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "rdns-whitelist-dir=${TMPDIR}/${TEST_NUM}-whitelist_rdns.d" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
