# This test checks if spamdyke will allow a message to be delivered after
# graylisting has previously occurred.  The existing graylist file is
# stored in the 3.x directory structure, this test should move it to the 4.x
# directory structure.

export TCPREMOTEIP=0.0.0.0

FROM_USERNAME=test-${TEST_NUM}.${RANDOM}.${RANDOM}
FROM_ADDRESS=${FROM_USERNAME}@example.com

mkdir -p ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`/`echo $1 | sed -e "s/@.*//" | awk '{ print tolower($1) }'`
touch ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`/`echo $1 | sed -e "s/@.*//" | awk '{ print tolower($1) }'`/${FROM_ADDRESS}

cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --graylist-level always -g ${TMPDIR}/${TEST_NUM}-graylist.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --graylist-level always -g ${TMPDIR}/${TEST_NUM}-graylist.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  if [ -f ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`/`echo $1 | sed -e "s/@.*//" | awk '{ print tolower($1) }'`/example.com/${FROM_USERNAME} ]
  then
    if [ ! -f ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`/`echo $1 | sed -e "s/@.*//" | awk '{ print tolower($1) }'`/${FROM_ADDRESS} ]
    then
      outcome="success"
    else
      echo "${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`/`echo $1 | sed -e "s/@.*//" | awk '{ print tolower($1) }'`/${FROM_ADDRESS} should not exist!"
      outcome="failure"
    fi
  else
    echo "${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`/`echo $1 | sed -e "s/@.*//" | awk '{ print tolower($1) }'`/example.com/${FROM_USERNAME} does not exist!"
    outcome="failure"
  fi
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
