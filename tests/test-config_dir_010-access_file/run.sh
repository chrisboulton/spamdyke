# This test looks for a rejection when the remote server is not allowed to
# connect.

export TCPREMOTEIP=10.64.128.255

echo "10.64.128.255:deny" > ${TMPDIR}/${TEST_NUM}-access.txt
echo ":allow" >> ${TMPDIR}/${TEST_NUM}-access.txt

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com
echo "access-file=${TMPDIR}/${TEST_NUM}-access.txt" > ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "local-domains-file=${TMPDIR}/${TEST_NUM}-local_domains.txt" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example

touch ${TMPDIR}/${TEST_NUM}-local_domains.txt

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} --log-target stderr --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} --log-target stderr --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR: Option not allowed in configuration file, found in file ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example on line 1: access-file" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "ERROR: Option not allowed in configuration file, found in file ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example on line 2: local-domains-file" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    outcome="success"
  else
    echo Filter failure - tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
