# This test looks for a rejection because the incoming rDNS name doesn't
# resolve.  The option to check the rDNS name should come from a config-dir
# that matches the full sender address.
# Multiple matches are possible and the all-sender option is not given.

export TCPREMOTEIP=${TESTSD_UNRESOLVABLE_RDNS_IP}

FROM_USERNAME=test-${TEST_NUM}.${RANDOM}.${RANDOM}
FROM_ADDRESS=${FROM_USERNAME}@example.com

rdns_path=""
for segment in `${DNSPTR_PATH} ${TESTSD_UNRESOLVABLE_RDNS_IP} | sed -e "s/\./ /g"`
do
  if [ "${rdns_path}" == "" ]
  then
    rdns_path=${segment}
  else
    rdns_path=${segment}/${rdns_path}
  fi
done

rdns_dir=`echo ${rdns_path} | sed -e "s/\/[^/]*$//g"`

recipient_dir=""
for segment in `echo $1 | awk '{ print tolower($1) }' | sed -e "s/[^@]*@//" -e "s/\./ /g"`
do
  if [ "${recipient_dir}" == "" ]
  then
    recipient_dir=${segment}
  else
    recipient_dir=${segment}/${recipient_dir}
  fi
done

recipient_dir=${recipient_dir}/_at_
recipient_path=${recipient_dir}/`echo $1 | awk '{ print tolower($1) }' | sed -e "s/@.*//"`

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_rdns_/${rdns_path}/_sender_/com/example/_at_
mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_recipient_/${recipient_path}/_sender_/com/example/_at_

echo rejection-text-unresolvable-rdns=Foo Bar Baz Qux > ${TMPDIR}/${TEST_NUM}-config.d/_rdns_/${rdns_path}/_sender_/com/example/_at_/${FROM_USERNAME}
echo reject-unresolvable-rdns > ${TMPDIR}/${TEST_NUM}-config.d/_recipient_/${recipient_path}/_sender_/com/example/_at_/${FROM_USERNAME}

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --config-dir-search all-sender ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d --config-dir-search all-sender ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "421 Foo Bar Baz Qux" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
