# This test checks if spamdyke will graylist domains that have proper graylist
# directories setup (activated) when the "no-graylist" option is used and the remote
# server's rDNS name is listed in a "grayblacklist" file.

export TCPREMOTEIP=${TESTSD_RDNS_IP}

${DNSPTR_PATH} ${TESTSD_RDNS_IP} > ${TMPDIR}/${TEST_NUM}-grayblacklist_rdns.txt

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

mkdir -p ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} --graylist-level only --graylist-dir ${TMPDIR}/${TEST_NUM}-graylist.d --graylist-exception-rdns-file ${TMPDIR}/${TEST_NUM}-grayblacklist_rdns.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} --graylist-level only --graylist-dir ${TMPDIR}/${TEST_NUM}-graylist.d --graylist-exception-rdns-file ${TMPDIR}/${TEST_NUM}-grayblacklist_rdns.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "421 Your address has been graylisted. Try again later." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
