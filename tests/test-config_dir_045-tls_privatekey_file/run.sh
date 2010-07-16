# This test looks for a STARTTLS offer, starts TLS and checks that spamdyke is
# doing the TLS with separate certificate and key files, not qmail.

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

mkdir -p ${TMPDIR}/${TEST_NUM}-logs

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com
echo "full-log-dir=${TMPDIR}/${TEST_NUM}-logs" > ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "smtp-auth-command=${AUTH_CMDLINE}" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "tls-certificate-file=${CERTDIR}/separate_no_passphrase/server.crt.pem" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "tls-privatekey-file=${CERTDIR}/separate_no_passphrase/server.key.pem" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example

cat input.txt | sed -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" -e "s/TO_ADDRESS/$1/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: Option not allowed in configuration file, found in file ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example on line 4: tls-privatekey-file" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
