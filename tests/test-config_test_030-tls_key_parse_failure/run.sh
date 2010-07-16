# This test looks for an error message from the config-test when it finds a
# TLS private key that cannot be parsed.

touch ${TMPDIR}/${TEST_NUM}-key.pem

echo "${SPAMDYKE_PATH} -ldebug --tls-certificate-file ${CERTDIR}/separate_passphrase_foobar/server.crt.pem --tls-privatekey-file ${TMPDIR}/${TEST_NUM}-key.pem --tls-privatekey-password foobar --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -ldebug --tls-certificate-file ${CERTDIR}/separate_passphrase_foobar/server.crt.pem --tls-privatekey-file ${TMPDIR}/${TEST_NUM}-key.pem --tls-privatekey-password foobar --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "unable to load or decrypt SSL/TLS private key from file" ${TMPDIR}/${TEST_NUM}-output.txt`
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
