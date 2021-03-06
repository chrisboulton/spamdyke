# This test looks for a rejection because the incoming rDNS name doesn't
# resolve..  The option to check the rDNS name should come from a config-dir
# that matches the full sender domain (no username).

export TCPREMOTEIP=${TESTSD_UNRESOLVABLE_RDNS_IP}

FROM_USERNAME=test-${TEST_NUM}.${RANDOM}.${RANDOM}
FROM_ADDRESS=${FROM_USERNAME}@internal.mail.example.com

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example/mail

echo policy-url=foo > ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example/mail/foo
echo policy-url=bar > ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example/mail/bar
echo policy-url=baz > ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example/mail/baz
echo reject-unresolvable-rdns > ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example/mail/internal

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "421 Refused. Your reverse DNS entry does not resolve." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "See: " ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ -z "${output}" ]
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
