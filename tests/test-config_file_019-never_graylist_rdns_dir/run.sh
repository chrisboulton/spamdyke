# This test checks if spamdyke will skip graylisting for remote senders whose
# rDNS names are listed in the "graywhitelist" directory structure.

export TCPREMOTEIP=${TESTSD_RDNS_IP}

INCOMING_DOMAIN=`${DNSPTR_PATH} ${TESTSD_RDNS_IP} | ${DOMAINSPLIT_PATH}`
mkdir -p ${TMPDIR}/${TEST_NUM}-graywhitelist.d/`${DOMAIN2PATH_PATH} -d ${INCOMING_DOMAIN}`
touch ${TMPDIR}/${TEST_NUM}-graywhitelist.d/`${DOMAIN2PATH_PATH} ${INCOMING_DOMAIN}`

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

echo "graylist-dir=${TMPDIR}/${TEST_NUM}-graylist.d" > ${TMPDIR}/${TEST_NUM}-config.txt
echo "graylist-exception-rdns-dir=${TMPDIR}/${TEST_NUM}-graywhitelist.d" >> ${TMPDIR}/${TEST_NUM}-config.txt

mkdir -p ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
