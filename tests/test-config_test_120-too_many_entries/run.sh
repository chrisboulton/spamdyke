# This test runs spamdyke with too many -entry options and checks for a warning
# from spamdyke to use a -file option instead.

i=0
while [ ${i} -lt 256 ]
do
  echo ip-whitelist-entry=192.168.1.${i} >> ${TMPDIR}/${TEST_NUM}-config.txt
  i=$[${i}+1]
done

echo "${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt -lverbose --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt -lverbose --config-test ${QMAIL_CMDLINE} > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "WARNING: ip-whitelist-entry is used 256 times; to increase efficiency, consider moving those values to a file instead." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
