# This test looks for a message from the config-test when it finds the
# relay-level option is "block-all" and the required option "local-domains-file"
# is given.

touch ${TMPDIR}/${TEST_NUM}-local_domains.txt

echo "${SPAMDYKE_PATH} --config-test --relay-level block-all --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} --config-test --relay-level block-all --local-domains-file ${TMPDIR}/${TEST_NUM}-local_domains.txt ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR(relay-level): The \"relay-level\" option is \"block-all\" but no local domains were given with \"local-domains-entry\" or \"local-domains-file\". The \"relay-level\" option will be ignored." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
