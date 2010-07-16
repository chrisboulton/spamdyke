# This test puts an unacceptable option in a configuration dir file and looks
# for the correct error message.

TO_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com
FROM_ADDRESS=bar@foo.com

touch ${TMPDIR}/${TEST_NUM}-keywords.txt
mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/foo/_at_/
echo "ip-in-rdns-blacklist-keyword-file=${TMPDIR}/${TEST_NUM}-keywords.txt" > ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/foo/_at_/bar

cat input.txt | sed -e "s/TARGET_EMAIL/${TO_ADDRESS}/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt

echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: Unknown configuration file option in file" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
