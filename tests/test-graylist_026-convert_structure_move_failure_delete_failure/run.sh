# This test checks if spamdyke will allow a message to be delivered after
# graylisting has previously occurred.  The existing graylist file is
# stored in the 3.x directory structure, this test should try to move it to
# the 4.x directory structure.  The existing file cannot be moved or deleted,
# so graylisting should fail.

if [ "${UID}" == "0" ]
then
  export TCPREMOTEIP=0.0.0.0

  FROM_USERNAME=test-${TEST_NUM}.${RANDOM}.${RANDOM}
  FROM_ADDRESS=${FROM_USERNAME}@example.com

  mkdir -p ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`/`echo $1 | sed -e "s/@.*//" | awk '{ print tolower($1) }'`
  mkdir -p ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`/`echo $1 | sed -e "s/@.*//" | awk '{ print tolower($1) }'`/example.com
  touch ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`/`echo $1 | sed -e "s/@.*//" | awk '{ print tolower($1) }'`/${FROM_ADDRESS}
  chmod 000 ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`/`echo $1 | sed -e "s/@.*//" | awk '{ print tolower($1) }'`/${FROM_ADDRESS}

  cat input.txt | sed -e "s/TEST_NUM/${TEST_NUM}/g" -e "s/TARGET_EMAIL/$1/g" -e "s/FROM_ADDRESS/${FROM_ADDRESS}/g" > ${TMPDIR}/${TEST_NUM}-input.txt

  pushd ..
  su $4 ./subrun $1 $2 $3 $4 $5 test-${TEST_NUM}-*
  popd

  output=`grep -E "250 ok [0-9]* qp [0-9]*" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    output=`grep "ERROR: unable to move file " ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ ! -z "${output}" ]
    then
      output=`grep "ERROR: unable to remove file " ${TMPDIR}/${TEST_NUM}-output.txt`
      if [ ! -z "${output}" ]
      then
        if [ ! -f ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`/`echo $1 | sed -e "s/@.*//" | awk '{ print tolower($1) }'`/example.com/${FROM_USERNAME} ]
        then
          if [ -f ${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`/`echo $1 | sed -e "s/@.*//" | awk '{ print tolower($1) }'`/${FROM_ADDRESS} ]
          then
            outcome="success"
          else
            echo "${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`/`echo $1 | sed -e "s/@.*//" | awk '{ print tolower($1) }'`/${FROM_ADDRESS} does not exist!"
            outcome="failure"
          fi
        else
          echo "${TMPDIR}/${TEST_NUM}-graylist.d/`echo $1 | sed -e "s/[^@]*@//" | awk '{ print tolower($1) }'`/`echo $1 | sed -e "s/@.*//" | awk '{ print tolower($1) }'`/example.com/${FROM_USERNAME} should not exist!"
          outcome="failure"
        fi
      else
        echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
        cat ${TMPDIR}/${TEST_NUM}-output.txt

        outcome="failure"
      fi
    else
      echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
      cat ${TMPDIR}/${TEST_NUM}-output.txt

      outcome="failure"
    fi
  else
    echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --log-target stderr --graylist-level always -g ${TMPDIR}/${TEST_NUM}-graylist.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
  ${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} --log-target stderr --graylist-level always -g ${TMPDIR}/${TEST_NUM}-graylist.d ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1
fi
