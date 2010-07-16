# This test starts TLS without sending any data to test if spamdyke will
# timeout or wait forever for SSL data.

export TCPREMOTEIP=0.0.0.0

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt

echo "${SENDRECV_PATH} -t 30 -S -r 421 -- ${SPAMDYKE_PATH} --tls-level smtp --tls-certificate ../certificates/combined_no_passphrase/server.pem -T 10 ${SMTPDUMMY_PATH} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -S -r 421 -- ${SPAMDYKE_PATH} --tls-level smtp --tls-certificate ../certificates/combined_no_passphrase/server.pem -T 10 ${SMTPDUMMY_PATH} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "421 Timeout. Talk faster next time." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
