# This test runs spamdyke with a large file that could be more efficiently
# replaced with a directory structure and looks for a warning.

i=0
while [ ${i} -lt 500 ]
do
  echo domain${i}.com >> ${TMPDIR}/${TEST_NUM}-rdns_blacklist.txt
  i=$[${i}+1]
done

echo "${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --rdns-blacklist-file ${TMPDIR}/${TEST_NUM}-rdns_blacklist.txt --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} --rdns-blacklist-file ${TMPDIR}/${TEST_NUM}-rdns_blacklist.txt --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "WARNING(rdns-blacklist-file): File length is inefficient; consider using a directory structure instead: ${TMPDIR}/${TEST_NUM}-rdns_blacklist.txt" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
