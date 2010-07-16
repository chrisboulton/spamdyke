# This test looks for a rejection because the incoming rDNS name doesn't
# resolve.  The option to check the rDNS name should come from a config-dir
# that matches the 4 octets of the IP address and the full recipient address.
# Multiple matches are possible but the all-ip option is not given.

export TCPREMOTEIP=${TESTSD_UNRESOLVABLE_RDNS_IP}

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

octets=`echo ${TESTSD_UNRESOLVABLE_RDNS_IP} | sed -e "s/\./ /g"`
octet_1=`echo ${octets} | awk '{ print $1 }'`
octet_2=`echo ${octets} | awk '{ print $2 }'`
octet_3=`echo ${octets} | awk '{ print $3 }'`
octet_4=`echo ${octets} | awk '{ print $4 }'`

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

mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_recipient_/${recipient_path}/_ip_/${octet_1}/${octet_2}/${octet_3}
mkdir -p ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example/_ip_/${octet_1}/${octet_2}/${octet_3}

echo rejection-text-unresolvable-rdns=Foo Bar Baz Qux > ${TMPDIR}/${TEST_NUM}-config.d/_sender_/com/example/_ip_/${octet_1}/${octet_2}/${octet_3}/${octet_4}
echo reject-unresolvable-rdns > ${TMPDIR}/${TEST_NUM}-config.d/_recipient_/${recipient_path}/_ip_/${octet_1}/${octet_2}/${octet_3}/${octet_4}

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --config-dir ${TMPDIR}/${TEST_NUM}-config.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "421 Foo Bar Baz Qux" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ -z "${output}" ]
then
  outcome="success"
else
  echo Filter failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
