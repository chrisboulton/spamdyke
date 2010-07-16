# This test delivers a small test message and looks for the correct qmail status
# code and a log message in both syslog and stderr.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --log-target stderr --log-target syslog -l ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --log-target stderr --log-target syslog -l ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "from: ${FROM_ADDRESS}" ${TMPDIR}/${TEST_NUM}-output.txt | grep ALLOWED`
  if [ ! -z "${output}" ]
  then
    output=`grep "from: ${FROM_ADDRESS}" ${TMPDIR}/${TEST_NUM}-output.txt | grep -E "spamdyke\[[0-9]*\]: spamdyke\[[0-9]\]: ALLOWED"`
    if [ -z "${output}" ]
    then
      echo Sleeping 5 seconds so syslogd can write the log entry.
      sleep 5
      output=`grep "from: ${FROM_ADDRESS}" /var/log/maillog | grep ALLOWED`
      if [ ! -z "${output}" ]
      then
        output=`grep "from: ${FROM_ADDRESS}" /var/log/maillog | grep -E "spamdyke\[[0-9]*\]: spamdyke\[[0-9]\]: ALLOWED"`
        if [ -z "${output}" ]
        then
          outcome="success"
        else
          echo Logging failure.  Check syslog.

          outcome="failure"
        fi
      else
        echo Logging failure.  Check syslog.

        outcome="failure"
      fi
    else
      echo OUTPUT IN ${TMPDIR}/${TEST_NUM}-output.txt:
      cat ${TMPDIR}/${TEST_NUM}-output.txt

      outcome="failure"
    fi
  else
    echo OUTPUT IN ${TMPDIR}/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo Delivery failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
