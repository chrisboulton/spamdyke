# This test looks for an idle timeout error when the buffer is constantly full
# due to a large message.

mkdir -p ${TMPDIR}/${TEST_NUM}-logs

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

echo "Building input file..."

cat input_1.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
i=0
while [ ${i} -lt 200 ]
do
  cat filler.txt >> ${TMPDIR}/${TEST_NUM}-filler.txt
  i=$[${i}+1]
done
i=0
while [ ${i} -lt 1000 ]
do
  cat ${TMPDIR}/${TEST_NUM}-filler.txt >> ${TMPDIR}/${TEST_NUM}-input.txt
  i=$[${i}+1]
done
cat input_2.txt >> ${TMPDIR}/${TEST_NUM}-input.txt

echo "${SPAMDYKE_PATH} -L ${TMPDIR}/${TEST_NUM}-logs -T 2 ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SPAMDYKE_PATH} -L ${TMPDIR}/${TEST_NUM}-logs -T 2 ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
