# This tests if BATV addresses are parsed correctly by the blacklist filter.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

echo $1 | awk '{ print tolower($1) }' > ${TMPDIR}/${TEST_NUM}-recipient_blacklist.txt

cat input.txt | sed -e "s/TARGET_EMAIL/prvs=`echo $1 | sed -e "s/\@.*//"`\/${RANDOM}${RANDOM}`echo $1 | sed -e "s/[^@]*//"`::/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} -S ${TMPDIR}/${TEST_NUM}-recipient_blacklist.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} -S ${TMPDIR}/${TEST_NUM}-recipient_blacklist.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "554 Refused. Mail is not being accepted at this address." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
