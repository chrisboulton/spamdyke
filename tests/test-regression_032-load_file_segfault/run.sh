# This test looks for segmentation fault when spamdyke loads RBLs from a file
# after some have been specified on the command line.

export TCPREMOTEIP=${TESTSD_RDNS_IP}

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

echo txt.dnsrbl > ${TMPDIR}/${TEST_NUM}-rbl.txt

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --dns-blacklist-entry a.example.com --dns-blacklist-entry b.example.com --dns-blacklist-entry c.example.com --dns-blacklist-entry d.example.com --dns-blacklist-entry e.example.com --dns-blacklist-entry f.example.com --dns-blacklist-entry g.example.com --dns-blacklist-entry h.example.com --dns-blacklist-entry i.example.com --dns-blacklist-entry j.example.com --dns-blacklist-file ${TMPDIR}/${TEST_NUM}-rbl.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --dns-blacklist-entry a.example.com --dns-blacklist-entry b.example.com --dns-blacklist-entry c.example.com --dns-blacklist-entry d.example.com --dns-blacklist-entry e.example.com --dns-blacklist-entry f.example.com --dns-blacklist-entry g.example.com --dns-blacklist-entry h.example.com --dns-blacklist-entry i.example.com --dns-blacklist-entry j.example.com --dns-blacklist-file ${TMPDIR}/${TEST_NUM}-rbl.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep -E "^221 " ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
