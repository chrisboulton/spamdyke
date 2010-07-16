# This test attempts to deliver a message to a remote recipient when the
# "relay-level" option is "block-all" and the client authenticates and the
# client is whitelisted and the RELAYCLIENT environment variable is set.

export TCPREMOTEIP=11.22.33.44
export RELAYCLIENT=""

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com
TO_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com
AUTH_DATA=`${SMTPAUTH_PLAIN_PATH} $2 $3 | tail -1 | awk '{ print $2 }'`

echo $1 | sed -e "s/[^@]*@//" > ${TMPDIR}/${TEST_NUM}-local_domains.txt
echo 11.22.33.44 > ${TMPDIR}/${TEST_NUM}-ip_whitelist.txt

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/${TO_ADDRESS}/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" -e "s/AUTH_DATA/${AUTH_DATA}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --relay-level block-all -d ${TMPDIR}/${TEST_NUM}-local_domains.txt --ip-whitelist-file ${TMPDIR}/${TEST_DIR}-ip_whitelist.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --relay-level block-all -d ${TMPDIR}/${TEST_NUM}-local_domains.txt --ip-whitelist-file ${TMPDIR}/${TEST_DIR}-ip_whitelist.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "554 Refused. Sending to remote addresses (relaying) is not allowed." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
