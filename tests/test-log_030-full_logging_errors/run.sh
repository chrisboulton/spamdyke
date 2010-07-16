# This test looks for errors to be logged to the full log as well as
# stderr.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com
TO_DOMAIN=`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`

mkdir ${TMPDIR}/${TEST_NUM}-logs
touch ${TMPDIR}/${TEST_NUM}-graylist

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --log-target stderr -g ${TMPDIR}/${TEST_NUM}-graylist --graylist-level always-create-dir -L ${TMPDIR}/${TEST_NUM}-logs ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --log-target stderr -g ${TMPDIR}/${TEST_NUM}-graylist --graylist-level always-create-dir -L ${TMPDIR}/${TEST_NUM}-logs ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: unable to create directory ${TMPDIR}/${TEST_NUM}-graylist/${TO_DOMAIN}: Not a directory" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "ERROR: unable to create directory ${TMPDIR}/${TEST_NUM}-graylist/${TO_DOMAIN}: Not a directory" ${TMPDIR}/${TEST_NUM}-logs/*`
  if [ ! -z "${output}" ]
  then
    outcome="success"
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
