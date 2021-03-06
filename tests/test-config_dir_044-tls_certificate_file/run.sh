# This test looks for a STARTTLS offer when qmail doesn't support it and
# starts TLS.

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com
echo "reject-empty-rdns=yes" > ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "smtp-auth-command=${AUTH_CMDLINE}" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "tls-certificate-file=${CERTDIR}/combined_no_passphrase/server.pem" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example

cat input.txt | sed -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" -e "s/TO_ADDRESS/$1/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: Option not allowed in configuration file, found in file ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example on line 3: tls-certificate-file" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
