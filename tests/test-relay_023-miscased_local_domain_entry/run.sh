# This test attempts to deliver a message to a local recipient when the
# "relay-level" option is "normal".

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

TO_DOMAIN=`echo $1 | sed -e "s/[^@]*@//" | awk '{ print toupper($1) }'`
echo ":allow" > ${TMPDIR}/${TEST_NUM}-access.txt

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --relay-level normal --access-file ${TMPDIR}/${TEST_NUM}-access.txt --local-domains-entry ${TO_DOMAIN} ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --relay-level normal --access-file ${TMPDIR}/${TEST_NUM}-access.txt --local-domains-entry ${TO_DOMAIN} ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
