# This test checks if spamdyke will graylist domains that have proper graylist
# directories setup (activated) when the "no-graylist" option is used and the remote
# IP address is listed in a "grayblacklist" file.

export TCPREMOTEIP=11.22.33.44

echo 11.22.33.44 > ${TMPDIR}/${TEST_NUM}-grayblacklist_ip.txt

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

echo "graylist-dir=${TMPDIR}/${TEST_NUM}-graylist.d" > ${TMPDIR}/${TEST_NUM}-config.txt
echo "graylist-level=only" >> ${TMPDIR}/${TEST_NUM}-config.txt
echo "graylist-exception-ip-file=${TMPDIR}/${TEST_NUM}-grayblacklist_ip.txt" >> ${TMPDIR}/${TEST_NUM}-config.txt

mkdir -p ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "421 Your address has been graylisted. Try again later." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
