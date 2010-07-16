# This test looks for a success message from the config-test when it finds the
# child process does not support TLS, spamdyke is compiled without TLS support
# and no SSL certificate is given.

child_cmd=`echo ${QMAIL_CMDLINE} | awk '{ print $1 }'`

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
    echo "${SPAMDYKE_PATH} -lverbose --tls-level=none --config-test ${QMAIL_CMDLINE} >> ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
    ${SPAMDYKE_PATH} -lverbose --tls-level=none --config-test ${QMAIL_CMDLINE} >> ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

    output=`grep "ERROR: ${child_cmd} does not appear to offer TLS support and spamdyke was not compiled with TLS support. The \"tls-type\" and \"tls-certificate-file\" options will be ignored. Please recompile spamdyke with TLS support." ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ ! -z "${output}" ]
    then
      output=`grep "ERROR(tls-certificate-file): TLS support is not compiled into this executable but a TLS certificate file was given anyway: ${CERTDIR}/combined_no_passphrase/server.pem" ${TMPDIR}/${TEST_NUM}-output.txt`
      if [ -z "${output}" ]
      then
        outcome="success"
      else
        echo Failure - tmp/${TEST_NUM}-output.txt:
        cat ${TMPDIR}/${TEST_NUM}-output.txt

        outcome="failure"
      fi
    else
      echo Failure - tmp/${TEST_NUM}-output.txt:
      cat ${TMPDIR}/${TEST_NUM}-output.txt

      outcome="failure"
    fi
  else
    echo Failure - tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo Failure - tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi

cp ${TMPDIR}/${TEST_NUM}-saved/* ${SPAMDYKE_DIR}
