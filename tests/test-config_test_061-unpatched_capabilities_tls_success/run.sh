# This test looks for a success message from the config-test when it finds the
# child process does not support TLS and the TLS option have been given.

child_cmd=`echo ${QMAIL_CMDLINE} | awk '{ print $1 }'`

echo "${SPAMDYKE_PATH} -ldebug --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -ldebug --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "SUCCESS: ${child_cmd} does not appear to offer TLS support. spamdyke will offer, intercept and decrypt TLS traffic." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
