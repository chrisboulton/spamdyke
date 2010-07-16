# This test looks for a STARTTLS offer when qmail doesn't support it and
# starts TLS.

mkdir -p ${TMPDIR}/${TEST_NUM}-logs

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

echo "Building input file..."

cat input_1.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
i=0
while [ ${i} -lt 1114 ]
do
  cat filler.txt >> ${TMPDIR}/${TEST_NUM}-input.txt
  i=$[${i}+1]
done
cat input_2.txt >> ${TMPDIR}/${TEST_NUM}-input.txt
i=0
while [ ${i} -lt 1013 ]
do
  cat filler.txt >> ${TMPDIR}/${TEST_NUM}-input.txt
  i=$[${i}+1]
done
cat input_3.txt >> ${TMPDIR}/${TEST_NUM}-input.txt
i=0
while [ ${i} -lt 2775 ]
do
  cat filler.txt >> ${TMPDIR}/${TEST_NUM}-input.txt
  i=$[${i}+1]
done
cat input_4.txt >> ${TMPDIR}/${TEST_NUM}-input.txt
i=0
while [ ${i} -lt 5004 ]
do
  cat filler.txt >> ${TMPDIR}/${TEST_NUM}-input.txt
  i=$[${i}+1]
done
cat input_5.txt >> ${TMPDIR}/${TEST_NUM}-input.txt
i=0
while [ ${i} -lt 205 ]
do
  cat filler_text.txt >> ${TMPDIR}/${TEST_NUM}-input.txt
  i=$[${i}+1]
done
cat input_6.txt >> ${TMPDIR}/${TEST_NUM}-input.txt

echo "${SENDRECV_PATH} -b 1024 -w 1 -t 30 -r 221 -- ${SPAMDYKE_PATH} -L ${TMPDIR}/${TEST_NUM}-logs --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -b 1024 -w 1 -t 30 -r 221 -- ${SPAMDYKE_PATH} -L ${TMPDIR}/${TEST_NUM}-logs --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
