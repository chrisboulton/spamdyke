# This test looks for a success message from the config-test when it finds a
# TLS certificate that can be loaded.

echo "${SPAMDYKE_PATH} -ldebug --tls-certificate-file ${CERTDIR}/separate_passphrase_foobar/server.crt.pem --tls-privatekey-file ${CERTDIR}/separate_passphrase_foobar/server.key.pem --tls-privatekey-password foobar --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -ldebug --tls-certificate-file ${CERTDIR}/separate_passphrase_foobar/server.crt.pem --tls-privatekey-file ${CERTDIR}/separate_passphrase_foobar/server.key.pem --tls-privatekey-password foobar --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "Certificate and key loaded; SSL/TLS library successfully initialized" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "SUCCESS: Tests complete. No errors detected." ${TMPDIR}/${TEST_NUM}-output.txt`
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
