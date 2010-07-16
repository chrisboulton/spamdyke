# This test whitelists the remote IP and checks for an AUTH banner.

export TCPREMOTEIP=11.22.33.44

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt

echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --ip-whitelist-entry 11.22.33.44 --smtp-auth-level always --smtp-auth-command \"${AUTH_CMDLINE}\" --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --ip-whitelist-entry 11.22.33.44 --smtp-auth-level always --smtp-auth-command "${AUTH_CMDLINE}" --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "AUTH LOGIN PLAIN" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
