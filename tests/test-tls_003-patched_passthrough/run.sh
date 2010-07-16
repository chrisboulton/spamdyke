# This test starts TLS with qmail, not spamdyke and checks that spamdyke
# passes the traffic through correctly.

if [ -f /var/qmail/control/servercert.pem ]
then
  mkdir -p ${TMPDIR}/${TEST_NUM}-logs

  cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
  echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -T 300 -L ${TMPDIR}/${TEST_NUM}-logs --smtp-auth-command \"${AUTH_CMDLINE}\" ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
  ${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -T 300 -L ${TMPDIR}/${TEST_NUM}-logs --smtp-auth-command "${AUTH_CMDLINE}" ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

  output=`grep "(TLS session started.)" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    output=`grep -E "^221" ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ ! -z "${output}" ]
    then
      output=`grep -E "^221" ${TMPDIR}/${TEST_NUM}-logs/*`
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
    echo Filter failure - tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo /var/qmail/control/servercert.pem does not exist.  Test failed.
  outcome="failure"
fi
