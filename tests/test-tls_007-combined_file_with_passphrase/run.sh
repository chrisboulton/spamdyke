# This test looks for a STARTTLS, starts TLS and checks that spamdyke is
# doing the TLS with a combined certificate and key file, with a passphrase.

mkdir -p ${TMPDIR}/${TEST_NUM}-logs

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -L ${TMPDIR}/${TEST_NUM}-logs --smtp-auth-command \"${AUTH_CMDLINE}\" --tls-certificate-file ${CERTDIR}/combined_passphrase_foobar/server.pem --tls-privatekey-password foobar ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -L ${TMPDIR}/${TEST_NUM}-logs --smtp-auth-command "${AUTH_CMDLINE}" --tls-certificate-file ${CERTDIR}/combined_passphrase_foobar/server.pem --tls-privatekey-password foobar ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "(TLS session started.)" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep -E "^221" ${TMPDIR}/${TEST_NUM}-logs/*`
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
