# This test starts TLS with qmail, not spamdyke and checks that spamdyke
# doesn't run post-connect filters.

if [ -f /var/qmail/control/servercert.pem ]
then
  FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

  echo ${FROM_ADDRESS} > ${TMPDIR}/${TEST_NUM}-sender_blacklist.txt

  cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
  echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -s ${TMPDIR}/${TEST_NUM}-sender_blacklist.txt --smtp-auth-command \"${AUTH_CMDLINE}\" ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
  ${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -s ${TMPDIR}/${TEST_NUM}-sender_blacklist.txt --smtp-auth-command "${AUTH_CMDLINE}" ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

  output=`grep "(TLS session started.)" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    output=`grep -E "^421" ${TMPDIR}/${TEST_NUM}-output.txt`
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
else
  echo /var/qmail/control/servercert.pem does not exist.  Test failed.
  outcome="failure"
fi
