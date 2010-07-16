# This test looks for a success message from the config-test when it finds the
# child process supports TLS, spamdyke is compiled without TLS support
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
    echo "${SPAMDYKE_PATH} -lverbose --tls-level=smtp --config-test ${QMAIL_CMDLINE} >> ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
    ${SPAMDYKE_PATH} -lverbose --tls-level=smtp --config-test ${QMAIL_CMDLINE} >> ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

    output=`grep "WARNING: ${child_cmd} appears to offer TLS support but spamdyke was not compiled with TLS support. spamdyke cannot use all of its filters unless it can intercept and decrypt the TLS traffic. Please recompile spamdyke with TLS support and use (or change) the \"tls-type\" and \"tls-certificate-file\" options. Unless it is recompiled with TLS support, the following spamdyke features will not function during TLS deliveries: graylisting, sender whitelisting, sender blacklisting, sender domain MX checking, DNS RHSBL checking for sender domains, recipient whitelisting, recipient blacklisting, limited number of recipients and full logging." ${TMPDIR}/${TEST_NUM}-output.txt`
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
