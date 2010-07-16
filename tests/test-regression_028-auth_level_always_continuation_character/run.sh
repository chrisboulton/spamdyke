# This test looks to see if spamdyke hides the AUTH advertisement from a patched
# qmail and correctly replaces it with its own AUTH advertisement.

export TCPREMOTEIP=0.0.0.0

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --smtp-auth-level always --smtp-auth-command \"${AUTH_CMDLINE}\" ${SMTPDUMMY_PATH} -a < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --smtp-auth-level always --smtp-auth-command "${AUTH_CMDLINE}" ${SMTPDUMMY_PATH} -a < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "250-AUTH LOGIN PLAIN" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "250-AUTH=LOGIN PLAIN" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ -z "${output}" ]
  then
    outcome="success"
  else
    echo Delivery failure - tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo Delivery failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
