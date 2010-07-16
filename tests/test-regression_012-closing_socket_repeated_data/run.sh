# This test looks for duplicated data at the end of the message when the sender
# disconnects after sending a burst large enough to fill the input buffer.

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

echo "Building input file..."

cat input_1.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
i=0
while [ ${i} -lt 1000 ]
do
  echo -n "00000000000000000000000000000000000000000000000000" >> ${TMPDIR}/${TEST_NUM}-input.txt
  i=$[${i}+1]
done
echo "00000000000000000000000000000000000000000000000000" >> ${TMPDIR}/${TEST_NUM}-input.txt
i=0
while [ ${i} -lt 1000 ]
do
  echo -n "00000000000000000000000000000000000000000000000000" >> ${TMPDIR}/${TEST_NUM}-input.txt
  i=$[${i}+1]
done
echo "00000000000000000000000000000000000000000000000000" >> ${TMPDIR}/${TEST_NUM}-input.txt
i=0
while [ ${i} -lt 1000 ]
do
  echo -n "00000000000000000000000000000000000000000000000000" >> ${TMPDIR}/${TEST_NUM}-input.txt
  i=$[${i}+1]
done
echo 1234 >> ${TMPDIR}/${TEST_NUM}-input.txt
cat input_2.txt >> ${TMPDIR}/${TEST_NUM}-input.txt

echo "${SPAMDYKE_PATH} -linfo --log-target stderr ${SMTPDUMMY_PATH} -o ${TMPDIR}/${TEST_NUM}-message_data.txt < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -linfo --log-target stderr ${SMTPDUMMY_PATH} -o ${TMPDIR}/${TEST_NUM}-message_data.txt < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "1234" ${TMPDIR}/${TEST_NUM}-message_data.txt | wc -l | awk '{ print $1 }'`
if [ "${output}" == "1" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-message_data.txt:
  cat ${TMPDIR}/${TEST_NUM}-message_data.txt

  outcome="failure"
fi
