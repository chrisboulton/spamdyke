# This test looks for a STARTTLS offer when qmail doesn't support it and
# starts TLS.

moved_cert=""
if [ -f /var/qmail/control/servercert.pem ]
then
  mv /var/qmail/control/servercert.pem /var/qmail/control/servercert.pem.bak
  moved_cert="true"
fi

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -r --smtp-auth-command \"${AUTH_CMDLINE}\" --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -r --smtp-auth-command "${AUTH_CMDLINE}" --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep -E "250.STARTTLS" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "(TLS session started.)" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
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

if [ ! -z "${moved_cert}" ]
then
  mv /var/qmail/control/servercert.pem.bak /var/qmail/control/servercert.pem
fi
