# This test checks spamdyke's response when qmail takes too long to respond to
# a command, spamdyke times out (idle-timeout-secs) and there is still data in
# the TLS buffer.  spamdyke should not lock up and loop infinitely.

mkdir -p ${TMPDIR}/${TEST_NUM}-logs

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

cat input_1.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
i=0
while [ ${i} -lt 500 ]
do
  cat input_2.txt >> ${TMPDIR}/${TEST_NUM}-input.txt
  i=$[${i}+1]
done
cat input_3.txt >> ${TMPDIR}/${TEST_NUM}-input.txt

echo "${SENDRECV_PATH} -B 65536 -b 65536 -w 0 -t 30 -r 221 -- ${SPAMDYKE_PATH} -T 5 --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem ${SMTPDUMMY_PATH} -d 10 < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -B 65536 -b 65536 -w 0 -t 30 -r 221 -- ${SPAMDYKE_PATH} -T 5 --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem ${SMTPDUMMY_PATH} -d 10 < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "421 Timeout. Talk faster next time." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
