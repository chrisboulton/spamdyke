# This test checks if spamdyke will graylist domains that have proper graylist
# directories setup (activated) when the "no-graylist" option is used and the remote
# server's rDNS name is listed in a "grayblacklist" directory structure.

export TCPREMOTEIP=${TESTSD_RDNS_IP}

INCOMING_DOMAIN=`${DNSPTR_PATH} ${TESTSD_RDNS_IP} | ${DOMAINSPLIT_PATH}`
mkdir -p ${TMPDIR}/${TEST_NUM}-grayblacklist.d/`${DOMAIN2PATH_PATH} -d ${INCOMING_DOMAIN}`
touch ${TMPDIR}/${TEST_NUM}-grayblacklist.d/`${DOMAIN2PATH_PATH} ${INCOMING_DOMAIN}`

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com
echo "graylist-dir=${TMPDIR}/${TEST_NUM}-graylist.d" > ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "graylist-level=only" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "graylist-exception-rdns-dir=${TMPDIR}/${TEST_NUM}-grayblacklist.d" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example

mkdir -p ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "421 Your address has been graylisted. Try again later." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
