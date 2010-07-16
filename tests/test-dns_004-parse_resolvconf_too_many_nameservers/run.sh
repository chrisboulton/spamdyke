# This test alters the /etc/resolv.conf file to set the nameserver to a known
# value and starts spamdyke to see if it parsed the file and queried the
# nameserver.  Too many nameserver values are given.  spamdyke should use only
# the first 16 and ignore the rest.

export TCPREMOTEIP=11.22.33.44
export NAMESERVER_IP_A=127.0.0.128
export NAMESERVER_IP_B=127.0.0.129
export NAMESERVER_IP_C=127.0.0.130
export NAMESERVER_IP_D=127.0.0.131
export NAMESERVER_IP_E=127.0.0.132
export NAMESERVER_IP_F=127.0.0.133
export NAMESERVER_IP_G=127.0.0.134
export NAMESERVER_IP_H=127.0.0.135
export NAMESERVER_IP_I=127.0.0.136
export NAMESERVER_IP_J=127.0.0.137
export NAMESERVER_IP_K=127.0.0.138
export NAMESERVER_IP_L=127.0.0.139
export NAMESERVER_IP_M=127.0.0.140
export NAMESERVER_IP_N=127.0.0.141
export NAMESERVER_IP_O=127.0.0.142
export NAMESERVER_IP_P=127.0.0.143
export NAMESERVER_IP_Q=127.0.0.144
export NAMESERVER_IP_R=127.0.0.145

if [ -f /etc/resolv.conf ]
then
  cp /etc/resolv.conf ${TMPDIR}/${TEST_NUM}-resolv.conf.bak
fi
cat resolv_conf.txt | sed -e "s/NAMESERVER_IP_A/${NAMESERVER_IP_A}/g" -e "s/NAMESERVER_IP_B/${NAMESERVER_IP_B}/g" -e "s/NAMESERVER_IP_C/${NAMESERVER_IP_C}/g" -e "s/NAMESERVER_IP_D/${NAMESERVER_IP_D}/g" -e "s/NAMESERVER_IP_E/${NAMESERVER_IP_E}/g" -e "s/NAMESERVER_IP_F/${NAMESERVER_IP_F}/g" -e "s/NAMESERVER_IP_G/${NAMESERVER_IP_G}/g" -e "s/NAMESERVER_IP_H/${NAMESERVER_IP_H}/g" -e "s/NAMESERVER_IP_I/${NAMESERVER_IP_I}/g" -e "s/NAMESERVER_IP_J/${NAMESERVER_IP_J}/g" -e "s/NAMESERVER_IP_K/${NAMESERVER_IP_K}/g" -e "s/NAMESERVER_IP_L/${NAMESERVER_IP_L}/g" -e "s/NAMESERVER_IP_M/${NAMESERVER_IP_M}/g" -e "s/NAMESERVER_IP_N/${NAMESERVER_IP_N}/g" -e "s/NAMESERVER_IP_O/${NAMESERVER_IP_O}/g" -e "s/NAMESERVER_IP_P/${NAMESERVER_IP_P}/g" -e "s/NAMESERVER_IP_Q/${NAMESERVER_IP_Q}/g" -e "s/NAMESERVER_IP_R/${NAMESERVER_IP_R}/g" > /etc/resolv.conf 
cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "found nameserver at /etc/resolv.conf(1): ${NAMESERVER_IP_A}" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "found nameserver at /etc/resolv.conf(18): ${NAMESERVER_IP_R}" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    output=`grep "found nameserver: ${NAMESERVER_IP_A}:53" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
    if [ "${output}" == "1" ]
    then
      output=`grep "found nameserver: ${NAMESERVER_IP_R}:53" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
      if [ "${output}" == "0" ]
      then
        outcome="success"
      else
        echo CONTENTS OF /etc/resolv.conf:
        cat /etc/resolv.conf
        echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
        cat ${TMPDIR}/${TEST_NUM}-output.txt

        outcome="failure"
      fi
    else
      echo CONTENTS OF /etc/resolv.conf:
      cat /etc/resolv.conf
      echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
      cat ${TMPDIR}/${TEST_NUM}-output.txt

      outcome="failure"
    fi
  else
    echo CONTENTS OF /etc/resolv.conf:
    cat /etc/resolv.conf
    echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
    cat ${TMPDIR}/${TEST_NUM}-output.txt

    outcome="failure"
  fi
else
  echo CONTENTS OF /etc/resolv.conf:
  cat /etc/resolv.conf
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi

if [ -f ${TMPDIR}/${TEST_NUM}-resolv.conf.bak ]
then
  cp ${TMPDIR}/${TEST_NUM}-resolv.conf.bak /etc/resolv.conf
fi
