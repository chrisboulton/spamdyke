# This test looks for a message from the config-test when it finds the
# relay-level option is "normal" and the required options "local-domains-file"
# and "access-file" are given.

touch ${TMPDIR}/${TEST_NUM}-local_domains.txt
touch ${TMPDIR}/${TEST_NUM}-access_file.txt

echo "${SPAMDYKE_PATH} --config-test --relay-level normal --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt --access-file ${TMPDIR}/${TEST_NUM}-access_file.txt ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} --config-test --relay-level normal --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt --access-file ${TMPDIR}/${TEST_NUM}-access_file.txt ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR(relay-level): The \"relay-level\" option is \"normal\" but no local domains were given with \"local-domains-entry\" or \"local-domains-file\". The \"relay-level\" option will be ignored." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ -z "${output}" ]
then
  output=`grep "ERROR(relay-level): The \"relay-level\" option is \"normal\" but no access files were given with \"access-file\". The \"relay-level\" option will be ignored." ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ -z "${output}" ]
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
