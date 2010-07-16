# This test runs spamdyke with an integer value below its minimum and checks the
# error message.

i=0
while [ ${i} -lt 2 ]
do
  j=0
  while [ ${j} -lt 256 ]
  do
    k=0
    while [ ${k} -lt 256 ]
    do
      echo 10.${i}.${j}.${k} >> ${TMPDIR}/${TEST_NUM}-ip_blacklist.txt
      k=$[${k}+1]
    done
    j=$[${j}+1]
  done
  i=$[${i}+1]
done

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt

echo "${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --ip-blacklist-file ${TMPDIR}/${TEST_NUM}-ip_blacklist.txt -lverbose --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --ip-blacklist-file ${TMPDIR}/${TEST_NUM}-ip_blacklist.txt -lverbose --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: ignoring file content past line 65536: ${TMPDIR}/${TEST_NUM}-ip_blacklist.txt" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
