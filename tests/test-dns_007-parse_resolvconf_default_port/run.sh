# This test alters the /etc/resolv.conf file to set the nameserver to a known
# value and starts spamdyke to see if it parsed the file and queried the
# nameserver.  A default port value is given.  spamdyke should use the default
# port for any nameservers that don't have port numbers explicitly defined.

export TCPREMOTEIP=11.22.33.44
export IP_A=127.0.0.128
export PORT_A=5555
export NAMESERVER_IP_A=${IP_A}:${PORT_A}
export IP_B=127.0.0.129
export PORT_B=6666
export NAMESERVER_IP_B=${IP_B}.${PORT_B}
export NAMESERVER_IP_C=127.0.0.130
export PORT_DEFAULT=1234

if [ -f /etc/resolv.conf ]
then
  cp /etc/resolv.conf ${TMPDIR}/${TEST_NUM}-resolv.conf.bak
fi
cat resolv_conf.txt | sed -e "s/NAMESERVER_IP_A/${NAMESERVER_IP_A}/g" -e "s/NAMESERVER_IP_B/${NAMESERVER_IP_B}/g" -e "s/NAMESERVER_IP_C/${NAMESERVER_IP_C}/g" -e "s/NAMESERVER_IP_D/${NAMESERVER_IP_D}/g" -e "s/NAMESERVER_IP_E/${NAMESERVER_IP_E}/g" -e "s/NAMESERVER_IP_F/${NAMESERVER_IP_F}/g" -e "s/NAMESERVER_IP_G/${NAMESERVER_IP_G}/g" -e "s/NAMESERVER_IP_H/${NAMESERVER_IP_H}/g" -e "s/NAMESERVER_IP_I/${NAMESERVER_IP_I}/g" -e "s/NAMESERVER_IP_J/${NAMESERVER_IP_J}/g" -e "s/NAMESERVER_IP_K/${NAMESERVER_IP_K}/g" -e "s/NAMESERVER_IP_L/${NAMESERVER_IP_L}/g" -e "s/NAMESERVER_IP_M/${NAMESERVER_IP_M}/g" -e "s/NAMESERVER_IP_N/${NAMESERVER_IP_N}/g" -e "s/NAMESERVER_IP_O/${NAMESERVER_IP_O}/g" -e "s/NAMESERVER_IP_P/${NAMESERVER_IP_P}/g" -e "s/NAMESERVER_IP_Q/${NAMESERVER_IP_Q}/g" -e "s/NAMESERVER_IP_R/${NAMESERVER_IP_R}/g" -e "s/PORT_DEFAULT/${PORT_DEFAULT}/g" > /etc/resolv.conf 
cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "found nameserver at /etc/resolv.conf(1): ${NAMESERVER_IP_A}" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "found nameserver at /etc/resolv.conf(2): ${NAMESERVER_IP_B}" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    output=`grep "found nameserver at /etc/resolv.conf(3): ${NAMESERVER_IP_C}" ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ ! -z "${output}" ]
    then
      output=`grep "found resolver default port at /etc/resolv.conf(4): ${PORT_DEFAULT}" ${TMPDIR}/${TEST_NUM}-output.txt`
      if [ ! -z "${output}" ]
      then
        output=`grep "found nameserver: ${IP_A}:${PORT_A}" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
        if [ "${output}" == "1" ]
        then
          output=`grep "found nameserver: ${IP_B}:${PORT_B}" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
          if [ "${output}" == "1" ]
          then
            output=`grep "found nameserver: ${NAMESERVER_IP_C}:${PORT_DEFAULT}" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
            if [ "${output}" == "1" ]
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
