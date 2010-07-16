# This test delivers a small test message to two recipients, the first is
# whitelisted and the other is blacklisted.  The first should be accepted, the
# second should be blocked.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com
TO_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

echo $1 > ${TMPDIR}/${TEST_NUM}-recipient_whitelist.txt
echo ${TO_ADDRESS} > ${TMPDIR}/${TEST_NUM}-recipient_blacklist.txt

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL1/$1/g" -e "s/TARGET_EMAIL2/${TO_ADDRESS}/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -linfo --log-target stderr --recipient-whitelist-file ${TMPDIR}/${TEST_NUM}-recipient_whitelist.txt --recipient-blacklist-file ${TMPDIR}/${TEST_NUM}-recipient_blacklist.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -linfo --log-target stderr --recipient-whitelist-file ${TMPDIR}/${TEST_NUM}-recipient_whitelist.txt --recipient-blacklist-file ${TMPDIR}/${TEST_NUM}-recipient_blacklist.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep -i "ALLOWED from: ${FROM_ADDRESS} to: $1" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "DENIED_RECIPIENT_BLACKLISTED from: ${FROM_ADDRESS} to: ${TO_ADDRESS}" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    output=`grep "554 Refused. Mail is not being accepted at this address." ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ ! -z "${output}" ]
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
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
