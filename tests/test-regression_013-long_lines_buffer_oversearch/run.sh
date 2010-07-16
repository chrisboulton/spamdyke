# This test looks for corrupted data when the input comes in bursts with
# delays and fills the input buffer exactly.

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

echo "Building input file..."

cat input_1.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt

echo -n "abcdefghijklmnopqrstuvwxy" >> ${TMPDIR}/${TEST_NUM}-input.txt
echo "abcdefghijklmnopqrstuvwxy" >> ${TMPDIR}/${TEST_NUM}-input.txt

i=1
while [ ${i} -lt 1632 ]
do
  echo -n "0123456789" >> ${TMPDIR}/${TEST_NUM}-input.txt
  i=$[${i}+1]
done
echo "0123456789" >> ${TMPDIR}/${TEST_NUM}-input.txt

echo -n "abcdefghijklmnopqrstuvwxy" >> ${TMPDIR}/${TEST_NUM}-input.txt
echo "abcdefghijklmnopqrstuvwxy" >> ${TMPDIR}/${TEST_NUM}-input.txt

i=1
while [ ${i} -lt 1632 ]
do
  echo -n "0123456789" >> ${TMPDIR}/${TEST_NUM}-input.txt
  i=$[${i}+1]
done
echo "0123456789" >> ${TMPDIR}/${TEST_NUM}-input.txt

echo -n "abcdefghijklmnopqrstuvwxy" >> ${TMPDIR}/${TEST_NUM}-input.txt
echo "abcdefghijklmnopqrstuvwxy" >> ${TMPDIR}/${TEST_NUM}-input.txt

i=1
while [ ${i} -lt 1632 ]
do
  echo -n "0123456789" >> ${TMPDIR}/${TEST_NUM}-input.txt
  i=$[${i}+1]
done
echo "0123456789" >> ${TMPDIR}/${TEST_NUM}-input.txt

cat input_2.txt >> ${TMPDIR}/${TEST_NUM}-input.txt

echo "${SENDRECV_PATH} -b 16382 -W 1 -w 1 -t 30 -r 221 -- ${SPAMDYKE_PATH} -linfo --log-target stderr ${SMTPDUMMY_PATH} -o ${TMPDIR}/${TEST_NUM}-message_data.txt < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -b 16382 -W 1 -w 1 -t 30 -r 221 -- ${SPAMDYKE_PATH} -linfo --log-target stderr ${SMTPDUMMY_PATH} -o ${TMPDIR}/${TEST_NUM}-message_data.txt < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

sed -e "s/abcdefghijklmnopqrstuvwxy//g" -e "s/0123456789//g" ${TMPDIR}/${TEST_NUM}-message_data.txt > ${TMPDIR}/${TEST_NUM}-bad_message_data.txt
output=`ls -l ${TMPDIR}/${TEST_NUM}-bad_message_data.txt | awk '{ print $5 }'`
if [ "${output}" == "12" ]
then
  outcome="success"
else
  echo BAD DATA IN tmp/${TEST_NUM}-bad_message_data.txt:
  cat ${TMPDIR}/${TEST_NUM}-bad_message_data.txt

  outcome="failure"
fi
