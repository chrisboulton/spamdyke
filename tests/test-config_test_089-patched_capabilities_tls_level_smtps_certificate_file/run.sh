# This test looks for a success message from the config-test when it finds the
# child process supports TLS, SMTPS support is requested in spamdyke and
# an SSL certificate is given.

child_cmd=`echo ${QMAIL_CMDLINE} | awk '{ print $1 }'`

echo "${SPAMDYKE_PATH} -lverbose --tls-level=smtps --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -lverbose --tls-level=smtps --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "SUCCESS: ${child_cmd} appears to offer TLS support but spamdyke will intercept and decrypt the TLS traffic so all of its filters can operate." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "SUCCESS(tls-certificate-file): Opened for reading: ${CERTDIR}/combined_no_passphrase/server.pem" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    output=`grep "SUCCESS(tls-certificate-file): Certificate and key loaded; SSL/TLS library successfully initialized" ${TMPDIR}/${TEST_NUM}-output.txt`
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
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
