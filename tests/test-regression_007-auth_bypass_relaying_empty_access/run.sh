# This test authenticates using SMTP AUTH LOGIN and delivers a small message.
# It checks to make sure the relaying filter is bypassed, even when the access
# file is empty.

export TCPREMOTEIP=0.0.0.0

touch ${TMPDIR}/${TEST_NUM}-access.txt

TO_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com
FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com
AUTH_USERNAME=`${SMTPAUTH_LOGIN_PATH} $2 $3 | tail -3 | head -1 | awk '{ print $2 }'`
AUTH_PASSWORD=`${SMTPAUTH_LOGIN_PATH} $2 $3 | tail -1 | awk '{ print $2 }'`

touch ${TMPDIR}/${TEST_NUM}-local_domains.txt

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/" -e "s/TARGET_EMAIL/${TO_ADDRESS}/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" -e "s/AUTH_USERNAME/${AUTH_USERNAME}/g" -e "s/AUTH_PASSWORD/${AUTH_PASSWORD}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --access-file ${TMPDIR}/${TEST_NUM}-access.txt -d ${TMPDIR}/${TEST_NUM}-local_domains.txt --smtp-auth-command \"${AUTH_CMDLINE}\" ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --access-file ${TMPDIR}/${TEST_NUM}-access.txt -d ${TMPDIR}/${TEST_NUM}-local_domains.txt --smtp-auth-command "${AUTH_CMDLINE}" ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Delivery failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
