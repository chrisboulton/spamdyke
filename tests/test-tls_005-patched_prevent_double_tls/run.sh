# This test starts TLS with spamdyke when qmail supports it, then attempts to
# start TLS again and checks that spamdyke hides qmail's STARTTLS advertisement
# and blocks the second attempt.

if [ -f /var/qmail/control/servercert.pem ]
then
  cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
  echo "${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} --smtp-auth-command \"${AUTH_CMDLINE}\" --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
  ${SENDRECV_PATH} -t 30 -r 554 -- ${SPAMDYKE_PATH} --smtp-auth-command "${AUTH_CMDLINE}" --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

  output=`grep -E "250.STARTTLS" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
  if [ "${output}" == "1" ]
  then
    output=`grep -E "^220" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
    if [ "${output}" == "2" ]
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
