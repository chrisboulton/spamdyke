# This test unsets a value from a string array with multiple values.  The value
# should be removed.

export TCPREMOTEIP=11.22.33.44
export PRIMARY_NAMESERVER_IP_1=127.0.0.125
export PRIMARY_NAMESERVER_IP_2=127.0.0.126
export PRIMARY_NAMESERVER_IP_3=127.0.0.127
export BAD_NAMESERVER_IP=127.0.0.12

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com
echo dns-blacklist-entry=foo >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo dns-blacklist-entry=bar >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo dns-blacklist-entry=baz >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo dns-blacklist-entry=!qux >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example

cat input.txt | sed -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" -e "s/TARGET_EMAIL/$1/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "for 44.33.22.11.foo(A) to DNS server" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "for 44.33.22.11.bar(A) to DNS server" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    output=`grep "for 44.33.22.11.baz(A) to DNS server" ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ ! -z "${output}" ]
    then
      output=`grep "for 44.33.22.11.qux(A) to DNS server" ${TMPDIR}/${TEST_NUM}-output.txt`
      if [ -z "${output}" ]
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
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
