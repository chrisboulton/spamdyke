# This test looks to see if spamdyke passes TLS traffic through to qmail
# when qmail supports TLS but spamdyke does not.

if [ -f /var/qmail/control/servercert.pem ]
then
  cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
  mkdir -p ${TMPDIR}/${TEST_NUM}-logs

  mkdir -p ${TMPDIR}/${TEST_NUM}-saved
  cp ${SPAMDYKE_DIR}/Makefile ${SPAMDYKE_DIR}/config.h ${SPAMDYKE_DIR}/*.o ${SPAMDYKE_PATH} ${TMPDIR}/${TEST_NUM}-saved

  pushd ${SPAMDYKE_DIR}
  make distclean
  echo "./configure --disable-tls > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
  ./configure --disable-tls > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1
  echo "make >> ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
  make >> ${TMPDIR}/${TEST_NUM}-output.txt 2>&1
  popd

  if [ -x ${SPAMDYKE_PATH} ]
  then
    echo "${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} -v >> ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
    ${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} -v >> ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

    output=`grep "+TLS" ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ -z "${output}" ]
    then
      echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -L ${TMPDIR}/${TEST_NUM}-logs --tls-level smtp --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt >> ${TMPDIR}/${TEST_NUM}-output.txt"
      ${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -L ${TMPDIR}/${TEST_NUM}-logs --tls-level smtp --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt >> ${TMPDIR}/${TEST_NUM}-output.txt

      output=`grep -E "^221" ${TMPDIR}/${TEST_NUM}-output.txt`
      if [ ! -z "${output}" ]
      then
        output=`grep -E "^221" ${TMPDIR}/${TEST_NUM}-logs/*`
        if [ -z "${output}" ]
        then
          outcome="success"
        else
          echo Filter failure - tmp/${TEST_NUM}-output.txt:
          cat ${TMPDIR}/${TEST_NUM}-output.txt

          outcome="failure"
        fi
      else
        echo Filter failure - tmp/${TEST_NUM}-output.txt:
        cat ${TMPDIR}/${TEST_NUM}-output.txt

        outcome="failure"
      fi
    else
      echo Filter failure - tmp/${TEST_NUM}-output.txt:
      cat ${TMPDIR}/${TEST_NUM}-output.txt

      outcome="failure"
    fi
  else
    echo Filter failure - tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi

  cp ${TMPDIR}/${TEST_NUM}-saved/* ${SPAMDYKE_DIR}
else
  echo /var/qmail/control/servercert.pem does not exist.  Test failed.
  outcome="failure"
fi
