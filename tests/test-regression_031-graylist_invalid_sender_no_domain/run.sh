# This test checks if spamdyke will correctly create graylist folders and files
# when the sender address is invalid and contains an additional SIZE parameter.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=sender@
TO_USERNAME=test-${TEST_NUM}.${RANDOM}.${RANDOM}
TO_ADDRESS=${TO_USERNAME}@example.com

mkdir -p ${TMPDIR}/${TEST_NUM}-graylist.d

cat input.txt | sed -e "s/TARGET_EMAIL/${TO_ADDRESS}/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} --graylist-level always-create-dir -g ${TMPDIR}/${TEST_NUM}-graylist.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} --graylist-level always-create-dir -g ${TMPDIR}/${TEST_NUM}-graylist.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "421 Your address has been graylisted. Try again later." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  if [ -d ${TMPDIR}/${TEST_NUM}-graylist.d/example.com/${TO_USERNAME} ]
  then
    if [ -d ${TMPDIR}/${TEST_NUM}-graylist.d/example.com/${TO_USERNAME}/_none ]
    then
      if [ -f ${TMPDIR}/${TEST_NUM}-graylist.d/example.com/${TO_USERNAME}/_none/sender ]
      then
        outcome="success"
      else
        echo ${TMPDIR}/${TEST_NUM}-graylist.d/example.com/${TO_USERNAME}/_none/sender does not exist or is not a file
        outcome="failure"
      fi
    else
      echo ${TMPDIR}/${TEST_NUM}-graylist.d/example.com/${TO_USERNAME}/_none does not exist or is not a folder
      outcome="failure"
    fi
  else
    echo ${TMPDIR}/${TEST_NUM}-graylist.d/example.com/${TO_USERNAME} does not exist or is not a folder
    outcome="failure"
  fi
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
