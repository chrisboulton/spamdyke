# This test looks for a STARTTLS offer, starts TLS and checks that spamdyke is
# doing the TLS with separate certificate and key files, not qmail.

mkdir -p ${TMPDIR}/${TEST_NUM}-logs

echo "full-log-dir=${TMPDIR}/${TEST_NUM}-logs" > ${TMPDIR}/${TEST_NUM}-config.txt
echo "smtp-auth-command=${AUTH_CMDLINE}" >> ${TMPDIR}/${TEST_NUM}-config.txt
echo "tls-certificate-file=${CERTDIR}/separate_no_passphrase/server.crt.pem" >> ${TMPDIR}/${TEST_NUM}-config.txt
echo "tls-privatekey-file=${CERTDIR}/separate_no_passphrase/server.key.pem" >> ${TMPDIR}/${TEST_NUM}-config.txt

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

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
