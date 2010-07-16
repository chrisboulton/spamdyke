# This test starts TLS with qmail, not spamdyke and checks for the correct
# "encryption" value in the log message.

if [ -f /var/qmail/control/servercert.pem ]
then
  mkdir -p ${TMPDIR}/${TEST_NUM}-logs

  FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

  cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
  echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -l --log-target stderr -T 300 -L ${TMPDIR}/${TEST_NUM}-logs ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
  ${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -l --log-target stderr -T 300 -L ${TMPDIR}/${TEST_NUM}-logs ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

  output=`grep "encryption: TLS_PASSTHROUGH" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    outcome="success"
  else
    echo Filter failure - tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo /var/qmail/control/servercert.pem does not exist.  Test failed.
  outcome="failure"
fi
