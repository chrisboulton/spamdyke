# Too many primary nameserver values are given.  spamdyke should use only
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

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-server-ip-primary ${NAMESERVER_IP_A} --dns-server-ip-primary ${NAMESERVER_IP_B} --dns-server-ip-primary ${NAMESERVER_IP_C} --dns-server-ip-primary ${NAMESERVER_IP_D} --dns-server-ip-primary ${NAMESERVER_IP_E} --dns-server-ip-primary ${NAMESERVER_IP_F} --dns-server-ip-primary ${NAMESERVER_IP_G} --dns-server-ip-primary ${NAMESERVER_IP_H} --dns-server-ip-primary ${NAMESERVER_IP_I} --dns-server-ip-primary ${NAMESERVER_IP_J} --dns-server-ip-primary ${NAMESERVER_IP_K} --dns-server-ip-primary ${NAMESERVER_IP_L} --dns-server-ip-primary ${NAMESERVER_IP_M} --dns-server-ip-primary ${NAMESERVER_IP_N} --dns-server-ip-primary ${NAMESERVER_IP_O} --dns-server-ip-primary ${NAMESERVER_IP_P} --dns-server-ip-primary ${NAMESERVER_IP_Q} --dns-server-ip-primary ${NAMESERVER_IP_R} ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-server-ip-primary ${NAMESERVER_IP_A} --dns-server-ip-primary ${NAMESERVER_IP_B} --dns-server-ip-primary ${NAMESERVER_IP_C} --dns-server-ip-primary ${NAMESERVER_IP_D} --dns-server-ip-primary ${NAMESERVER_IP_E} --dns-server-ip-primary ${NAMESERVER_IP_F} --dns-server-ip-primary ${NAMESERVER_IP_G} --dns-server-ip-primary ${NAMESERVER_IP_H} --dns-server-ip-primary ${NAMESERVER_IP_I} --dns-server-ip-primary ${NAMESERVER_IP_J} --dns-server-ip-primary ${NAMESERVER_IP_K} --dns-server-ip-primary ${NAMESERVER_IP_L} --dns-server-ip-primary ${NAMESERVER_IP_M} --dns-server-ip-primary ${NAMESERVER_IP_N} --dns-server-ip-primary ${NAMESERVER_IP_O} --dns-server-ip-primary ${NAMESERVER_IP_P} --dns-server-ip-primary ${NAMESERVER_IP_Q} --dns-server-ip-primary ${NAMESERVER_IP_R} ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "found nameserver: ${NAMESERVER_IP_A}" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "found nameserver: ${NAMESERVER_IP_R}" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
  if [ "${output}" == "0" ]
  then
    output=`grep "found nameserver: ${NAMESERVER_IP_A}:53" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
    if [ "${output}" == "1" ]
    then
      output=`grep "found nameserver: ${NAMESERVER_IP_R}:53" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
      if [ "${output}" == "0" ]
      then
        outcome="success"
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
  echo OUTPUT IN tmp/${TEST_NUM}-output.txt:
  cat ${TMPDIR}/${TEST_NUM}-output.txt

  outcome="failure"
fi
