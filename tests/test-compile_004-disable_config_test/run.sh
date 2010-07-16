# This test configures and compiles spamdyke without configuration tests to
# check for compiler warnings and errors.

mkdir -p ${TMPDIR}/${TEST_NUM}-saved
cp ${SPAMDYKE_DIR}/Makefile ${SPAMDYKE_DIR}/config.h ${SPAMDYKE_DIR}/*.o ${SPAMDYKE_PATH} ${TMPDIR}/${TEST_NUM}-saved

pushd ${SPAMDYKE_DIR}
make distclean
echo "./configure --disable-config-test > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
./configure --disable-config-test > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1
echo "make >> ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
make >> ${TMPDIR}/${TEST_NUM}-output.txt 2>&1
popd

if [ -x ${SPAMDYKE_PATH} ]
then
  echo "${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} -v >> ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
  ${SENDRECV_PATH} -t 30 -- ${SPAMDYKE_PATH} -v >> ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

  output=`grep "Use -h for an option summary or see README.html for complete option details." ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    output=`grep " warning: " ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ -z "${output}" ]
    then
      output=`grep " error: " ${TMPDIR}/${TEST_NUM}-output.txt`
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
