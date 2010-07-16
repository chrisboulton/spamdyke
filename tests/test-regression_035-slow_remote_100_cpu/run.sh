# This test delivers a large message and simulates a slow connection from the
# remote server.  The "time" utility is used to determine if spamdyke uses
# 100% CPU or if it waits patiently.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
i=0
while [ ${i} -lt 600 ]
do
  cat filler.txt >> ${TMPDIR}/${TEST_NUM}-input.txt
  i=$[${i}+1]
done
cat input_end.txt >> ${TMPDIR}/${TEST_NUM}-input.txt

echo "${CPUTIME_PATH} ${SENDRECV_PATH} -t 60 -r 221 -b 17000 -d 0 -W 20 -w 0 -- ${SPAMDYKE_PATH} ${SMTPDUMMY_PATH} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${CPUTIME_PATH} ${SENDRECV_PATH} -t 60 -r 221 -b 17000 -d 0 -W 20 -w 0 -- ${SPAMDYKE_PATH} ${SMTPDUMMY_PATH} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

user_time=`grep -E "^cpu: 0" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${user_time}" ]
then
  sys_time=`grep -E "^sys: 0" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${sys_time}" ]
  then
    outcome="success"
  else
    echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
