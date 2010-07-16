# This test checks if spamdyke will graylist redeliveries when the minimum time
# has not passed.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com
echo "graylist-dir=${TMPDIR}/${TEST_NUM}-graylist.d" > ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "graylist-min-secs=600" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example
echo "graylist-level=always" >> ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example

mkdir -p ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`
touch ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`/${FROM_ADDRESS}

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "421 Your address has been graylisted. Try again later." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
