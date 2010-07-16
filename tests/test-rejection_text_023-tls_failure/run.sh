# This test attempts to start an SMTPS session but corrupts the
# TLS data and looks for a spamdyke rejection.

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -c -1 -- ${SPAMDYKE_PATH} --rejection-text-tls-failure \"Foo Bar Baz\" --tls-level smtp --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -c -1 -- ${SPAMDYKE_PATH} --rejection-text-tls-failure "Foo Bar Baz" --tls-level smtp --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "454 Foo Bar Baz" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "454 Failed to negotiate TLS connection." ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ -z "${output}" ]
  then
    outcome="success"
  else
    echo Filter failure - tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
