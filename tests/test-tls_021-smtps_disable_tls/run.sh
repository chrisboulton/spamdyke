# This test attempts to start an SMTPS session when spamdyke does not support
# TLS.  spamdyke should immediately disconnect.

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
    cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
    echo "${SENDRECV_PATH} -t 30 -r 221 -s -- ${SPAMDYKE_PATH} --tls-level smtps --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt >> ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
    ${SENDRECV_PATH} -t 30 -r 221 -s -- ${SPAMDYKE_PATH} --tls-level smtps --tls-certificate-file ${CERTDIR}/combined_no_passphrase/server.pem ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt >> ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

    output=`grep -E "^220 " ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ -z "${output}" ]
    then
      output=`grep "(SSL failed due to a syscall: , TLS EOF found)" ${TMPDIR}/${TEST_NUM}-output.txt`
      if [ ! -z "${output}" ]
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
