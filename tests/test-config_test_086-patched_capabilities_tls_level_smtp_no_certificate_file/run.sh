# This test looks for a success message from the config-test when it finds the
# child process supports TLS, TLS support is requested in spamdyke and
# no SSL certificate is given.

child_cmd=`echo ${QMAIL_CMDLINE} | awk '{ print $1 }'`

echo "${SPAMDYKE_PATH} -lverbose --tls-level=smtp --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -lverbose --tls-level=smtp --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "WARNING: ${child_cmd} appears to offer TLS support but spamdyke cannot use all of its filters unless it can intercept and decrypt the TLS traffic. Please use (or change) the \"tls-type\" and \"tls-certificate-file\" options. Otherwise, the following spamdyke features will not function during TLS deliveries: graylisting, sender whitelisting, sender blacklisting, sender domain MX checking, DNS RHSBL checking for sender domains, recipient whitelisting, recipient blacklisting, limited number of recipients and full logging." ${TMPDIR}/${TEST_NUM}-output.txt`
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
