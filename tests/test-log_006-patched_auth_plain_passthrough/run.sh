# This test authenticates using SMTP AUTH PLAIN without giving spamdyke the
# auth command to see if spamdyke will honor the qmail authentication and
# log the authenticated username.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com
AUTH_DATA=`${SMTPAUTH_PLAIN_PATH} $2 $3 | tail -1 | awk '{ print $2 }'`

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" -e "s/AUTH_DATA/${AUTH_DATA}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -r -l ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -r -l ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  echo Sleeping 5 seconds so syslogd can write the log entry.
  sleep 5
  output=`grep "from: ${FROM_ADDRESS}" /var/log/maillog | awk '{ print $16 }' | grep $2`
  if [ ! -z "${output}" ]
  then
    outcome="success"
  else
    echo Logging failure.  Check syslog.

    outcome="failure"
  fi
else
  echo Delivery failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
