# This test authenticates using SMTP AUTH LOGIN and delivers a small message.
# It checks to make sure the missing sender MX filter is bypassed.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@foo.example.com
AUTH_USERNAME=`${SMTPAUTH_LOGIN_PATH} $2 $3 | tail -3 | head -1 | awk '{ print $2 }'`
AUTH_PASSWORD=`${SMTPAUTH_LOGIN_PATH} $2 $3 | tail -1 | awk '{ print $2 }'`

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" -e "s/AUTH_USERNAME/${AUTH_USERNAME}/g" -e "s/AUTH_PASSWORD/${AUTH_PASSWORD}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --reject-missing-sender-mx --smtp-auth-command \"${AUTH_CMDLINE}\" ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --reject-missing-sender-mx --smtp-auth-command "${AUTH_CMDLINE}" ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Delivery failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
