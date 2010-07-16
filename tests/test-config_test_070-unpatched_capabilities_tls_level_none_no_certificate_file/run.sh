# This test looks for a success message from the config-test when it finds the
# child process does not support TLS, TLS support is disabled in spamdyke and no
# SSL certificate is given.

child_cmd=`echo ${QMAIL_CMDLINE} | awk '{ print $1 }'`

echo "${SPAMDYKE_PATH} -lverbose --tls-level=none --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -lverbose --tls-level=none --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "WARNING: ${child_cmd} does not appear to offer TLS support. Please use (or change) the \"tls-type\" and \"tls-certificate-file\" options so spamdyke can offer, intercept or decrypt TLS traffic." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "SUCCESS(tls-certificate-file): Opened for reading: ${CERTDIR}/combined_no_passphrase/server.pem" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ -z "${output}" ]
  then
    output=`grep "SUCCESS(tls-certificate-file): Certificate and key loaded; SSL/TLS library successfully initialized" ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ -z "${output}" ]
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
