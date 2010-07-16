# This test looks for a success message from the config-test when it finds an
# spamdyke binary that is somewhere along the PATH but not within a directory
# that is part of spamdyke's default PATH.

cp ${SPAMDYKE_PATH} ${TMPDIR}/${TEST_NUM}-spamdyke
chmod 755 ${TMPDIR}/${TEST_NUM}-spamdyke

old_path=${PATH}
export PATH=${PATH}:${TMPDIR}

echo "${TEST_NUM}-spamdyke --config-test -lverbose ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${TEST_NUM}-spamdyke --config-test -lverbose ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "SUCCESS: spamdyke binary (${TMPDIR}/${TEST_NUM}-spamdyke) is not owned by root and/or is not marked setuid." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi

export PATH=${old_path}
