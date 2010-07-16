# This test delivers a small test message from an empty sender with a SIZE
# parameter and looks to see if spamdyke correctly finds an empty address.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

echo "greeting-delay-secs=5" >> ${TMPDIR}/${TEST_NUM}-config.txt
echo "config-dir=${TMPDIR}/${TEST_NUM}-config.d" >> ${TMPDIR}/${TEST_NUM}-config.txt
mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example/_at_
echo "graylist-level=none" > ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example/_at_/foo

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: earlytalker filter cannot be activated after the start of the connection -- ignoring configuration option" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
