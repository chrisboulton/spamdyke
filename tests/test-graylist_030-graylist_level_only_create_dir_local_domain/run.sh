# This test checks if spamdyke will graylist local domains that have no domain
# directories when graylist-level is "only-create-dir" and the remote IP address is
# listed in a "grayblacklist" file.

export TCPREMOTEIP=11.22.33.44

echo 11.22.33.44 > ${TMPDIR}/${TEST_NUM}-grayblacklist_ip.txt
echo $1 | sed -e "s/[^@]*@//" > ${TMPDIR}/${TEST_NUM}-local_domains.txt

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

mkdir -p ${TMPDIR}/${TEST_NUM}-graylist.d

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} --graylist-level only-create-dir --graylist-dir ${TMPDIR}/${TEST_NUM}-graylist.d --graylist-exception-ip-file ${TMPDIR}/${TEST_NUM}-grayblacklist_ip.txt --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} --graylist-level only-create-dir --graylist-dir ${TMPDIR}/${TEST_NUM}-graylist.d --graylist-exception-ip-file ${TMPDIR}/${TEST_NUM}-grayblacklist_ip.txt --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "421 Your address has been graylisted. Try again later." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
