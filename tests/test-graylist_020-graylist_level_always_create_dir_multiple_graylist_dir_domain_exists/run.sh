# This test checks if spamdyke will graylist domains that have domain directories
# with multiple graylist directories.

export TCPREMOTEIP=0.0.0.0

FROM_ADDRESS=test-${TEST_NUM}.${RANDOM}.${RANDOM}@example.com

mkdir -p ${TMPDIR}/${TEST_NUM}-graylist_one.d/foo.`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`
mkdir -p ${TMPDIR}/${TEST_NUM}-graylist_one.d/bar.`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`
mkdir -p ${TMPDIR}/${TEST_NUM}-graylist_two.d/baz.`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`
mkdir -p ${TMPDIR}/${TEST_NUM}-graylist_two.d/qux.`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`
mkdir -p ${TMPDIR}/${TEST_NUM}-graylist_three.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`

cat input.txt | sed -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} --graylist-level always-create-dir -g ${TMPDIR}/${TEST_NUM}-graylist_one.d -g ${TMPDIR}/${TEST_NUM}-graylist_two.d -g ${TMPDIR}/${TEST_NUM}-graylist_three.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt"
${SENDRECV_PATH} -t 30 -r 421 -- ${SPAMDYKE_PATH} --graylist-level always-create-dir -g ${TMPDIR}/${TEST_NUM}-graylist_one.d -g ${TMPDIR}/${TEST_NUM}-graylist_two.d -g ${TMPDIR}/${TEST_NUM}-graylist_three.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt

output=`grep "421 Your address has been graylisted. Try again later." ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  if [ ! -d ${TMPDIR}/${TEST_NUM}-graylist_one.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'` ]
  then
    if [ ! -d ${TMPDIR}/${TEST_NUM}-graylist_two.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'` ]
    then
      outcome="success"
    else
      echo "${TMPDIR}/${TEST_NUM}-graylist_two.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'` exists but spamdyke should not have created it!"
      outcome="failure"
    fi
  else
    echo "${TMPDIR}/${TEST_NUM}-graylist_one.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'` exists but spamdyke should not have created it!"
    outcome="failure"
  fi
else
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
