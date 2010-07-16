# This test delivers a small test message and looks for the correct qmail status
# code.  spamdyke should log the entire SMTP conversation to a file.  The log
# should also contain debugging statements.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com
CHILD_CMD=`echo ${QMAIL_CMDLINE} | sed -e "s/ .*//"`

rm -rf ${TMPDIR}/${TEST_NUM}-logs
mkdir ${TMPDIR}/${TEST_NUM}-logs

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -L ${TMPDIR}/${TEST_NUM}-logs ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -L ${TMPDIR}/${TEST_NUM}-logs ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-logs/*`
  if [ ! -z "${output}" ]
  then
    output=`grep "preparing to start child process: ${child_cmd}" ${TMPDIR}/${TEST_NUM}-logs/*`
    if [ ! -z "${output}" ]
    then
      outcome="success"
    else
      echo Logging failure.  ${TMPDIR}/${TEST_NUM}-logs contains no files or log is incomplete.
      cat ${TMPDIR}/${TEST_NUM}-logs/*

      outcome="failure"
    fi
  else
    echo Logging failure.  ${TMPDIR}/${TEST_NUM}-logs contains no files or log is incomplete.
    cat ${TMPDIR}/${TEST_NUM}-logs/*

    outcome="failure"
  fi
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
