# This test checks the number of DNS retries: 3 to the primary, 3 more
# to both.

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

export TCPREMOTEIP=11.22.33.44

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com
echo log-level=excessive >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo log-target=stderr >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo dns-timeout-secs=10 >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo dns-server-ip-primary=127.0.0.1:52 >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo dns-server-ip=127.0.0.1:51 >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo dns-max-retries-primary=3 >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo dns-max-retries-total=6 >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example

cat input.txt | sed -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" -e "s/TARGET_EMAIL/$1/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: Option not allowed in configuration file, found in file ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example on line 6: dns-max-retries-primary" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "ERROR: Option not allowed in configuration file, found in file ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example on line 7: dns-max-retries-total" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    outcome="success"
  else
    echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
