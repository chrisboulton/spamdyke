# This test looks for an error message from the config-test when it finds a
# TLS certificate and private key that do not match.

echo "${SPAMDYKE_PATH} -ldebug --tls-certificate-file ${CERTDIR}/separate_passphrase_foobar/server.crt.pem --tls-privatekey-file ${CERTDIR}/separate_passphrase_foobar/server.key.pem --tls-privatekey-password badpassword --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -ldebug --tls-certificate-file ${CERTDIR}/separate_passphrase_foobar/server.crt.pem --tls-privatekey-file ${CERTDIR}/separate_passphrase_foobar/server.key.pem --tls-privatekey-password badpassword --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "incorrect SSL/TLS private key password or SSL/TLS certificate/privatekey mismatch" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "ERROR: Tests complete. Errors detected." ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    outcome="success"
  else
    echo Failure - tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
