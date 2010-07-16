# This test looks for a rejection because the remote server sends data before
# the SMTP greeting banner is received (earlytalker).

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com
echo "greeting-delay-secs=0" > ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -d 0 -- ${SPAMDYKE_PATH} -e 10 --config-dir ${TMPDIR}/${TEST_NUM}-config.d --log-target stderr -lexcessive ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -d 0 -- ${SPAMDYKE_PATH} -e 10 --config-dir ${TMPDIR}/${TEST_NUM}-config.d --log-target stderr -lexcessive ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "554 Refused. You are not following the SMTP protocol." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ -z "${output}" ]
then
  output=`grep "FILTER_EARLYTALKER delay: 10" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    outcome="success"
  else
    echo Filter failure - tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
