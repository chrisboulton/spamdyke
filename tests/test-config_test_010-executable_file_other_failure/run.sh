# This test looks for a failure message from the config-test when it finds a
# non-executable file owned by the user's secondary group.

touch ${TMPDIR}/${TEST_NUM}-hostname
chown 65535 ${TMPDIR}/${TEST_NUM}-hostname
chgrp 65535 ${TMPDIR}/${TEST_NUM}-hostname
chmod 000 ${TMPDIR}/${TEST_NUM}-hostname

echo "${SPAMDYKE_PATH} -ldebug --hostname-command ${TMPDIR}/${TEST_NUM}-hostname --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} -ldebug --hostname-command ${TMPDIR}/${TEST_NUM}-hostname --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "File is not executable: ${TMPDIR}/${TEST_NUM}-hostname: \"Other\" permissions apply but \"other\" executable bit is not set" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "ERROR: Tests complete. Errors detected." ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    outcome="success"
  else
    echo Failure - tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
