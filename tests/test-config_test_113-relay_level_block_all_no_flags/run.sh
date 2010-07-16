# This test looks for a message from the config-test when it finds the
# relay-level option is "block-all" but the required option "local-domains-file"
# is not given.

echo "${SPAMDYKE_PATH} --config-test --relay-level block-all ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SPAMDYKE_PATH} --config-test --relay-level block-all ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "ERROR(relay-level): The \"relay-level\" option is \"block-all\" but no local domains were given with \"local-domains-entry\" or \"local-domains-file\". The \"relay-level\" option will be ignored." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
