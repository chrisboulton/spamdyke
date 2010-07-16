# Too many nameserver values are given.  spamdyke should use only
# the first 16 and ignore the rest.

export TCPREMOTEIP=11.22.33.44
export NAMESERVER_IP_PRIMARY_A=127.0.0.1:5454
export NAMESERVER_IP_PRIMARY_B=127.0.0.1:5455
export NAMESERVER_IP_PRIMARY_C=127.0.0.1:5456
export NAMESERVER_IP_PRIMARY_D=127.0.0.1:5457
export NAMESERVER_IP_PRIMARY_E=127.0.0.1:5458
export NAMESERVER_IP_PRIMARY_F=127.0.0.1:5459
export NAMESERVER_IP_PRIMARY_G=127.0.0.1:5460
export NAMESERVER_IP_PRIMARY_H=127.0.0.1:5461
export NAMESERVER_IP_PRIMARY_I=127.0.0.1:5462
export NAMESERVER_IP_PRIMARY_J=127.0.0.1:5463
export NAMESERVER_IP_PRIMARY_K=127.0.0.1:5464
export NAMESERVER_IP_PRIMARY_L=127.0.0.1:5465
export NAMESERVER_IP_PRIMARY_M=127.0.0.1:5466
export NAMESERVER_IP_PRIMARY_N=127.0.0.1:5467
export NAMESERVER_IP_PRIMARY_O=127.0.0.1:5468
export NAMESERVER_IP_PRIMARY_P=127.0.0.1:5469
export NAMESERVER_IP_PRIMARY_Q=127.0.0.1:5470
export NAMESERVER_IP_PRIMARY_R=127.0.0.1:5471
export NAMESERVER_IP_SECONDARY_A=127.0.0.1:5472
export NAMESERVER_IP_SECONDARY_B=127.0.0.1:5473
export NAMESERVER_IP_SECONDARY_C=127.0.0.1:5474
export NAMESERVER_IP_SECONDARY_D=127.0.0.1:5475
export NAMESERVER_IP_SECONDARY_E=127.0.0.1:5476
export NAMESERVER_IP_SECONDARY_F=127.0.0.1:5477
export NAMESERVER_IP_SECONDARY_G=127.0.0.1:5478
export NAMESERVER_IP_SECONDARY_H=127.0.0.1:5479
export NAMESERVER_IP_SECONDARY_I=127.0.0.1:5480
export NAMESERVER_IP_SECONDARY_J=127.0.0.1:5481
export NAMESERVER_IP_SECONDARY_K=127.0.0.1:5482
export NAMESERVER_IP_SECONDARY_L=127.0.0.1:5483
export NAMESERVER_IP_SECONDARY_M=127.0.0.1:5484
export NAMESERVER_IP_SECONDARY_N=127.0.0.1:5485
export NAMESERVER_IP_SECONDARY_O=127.0.0.1:5486
export NAMESERVER_IP_SECONDARY_P=127.0.0.1:5487
export NAMESERVER_IP_SECONDARY_Q=127.0.0.1:5488
export NAMESERVER_IP_SECONDARY_R=127.0.0.1:5489

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 60 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_A} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_B} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_C} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_D} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_E} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_F} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_G} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_H} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_I} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_J} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_K} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_L} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_M} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_N} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_O} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_P} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_Q} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_R} --dns-server-ip ${NAMESERVER_IP_SECONDARY_A} --dns-server-ip ${NAMESERVER_IP_SECONDARY_B} --dns-server-ip ${NAMESERVER_IP_SECONDARY_C} --dns-server-ip ${NAMESERVER_IP_SECONDARY_D} --dns-server-ip ${NAMESERVER_IP_SECONDARY_E} --dns-server-ip ${NAMESERVER_IP_SECONDARY_F} --dns-server-ip ${NAMESERVER_IP_SECONDARY_G} --dns-server-ip ${NAMESERVER_IP_SECONDARY_H} --dns-server-ip ${NAMESERVER_IP_SECONDARY_I} --dns-server-ip ${NAMESERVER_IP_SECONDARY_J} --dns-server-ip ${NAMESERVER_IP_SECONDARY_K} --dns-server-ip ${NAMESERVER_IP_SECONDARY_L} --dns-server-ip ${NAMESERVER_IP_SECONDARY_M} --dns-server-ip ${NAMESERVER_IP_SECONDARY_N} --dns-server-ip ${NAMESERVER_IP_SECONDARY_O} --dns-server-ip ${NAMESERVER_IP_SECONDARY_P} --dns-server-ip ${NAMESERVER_IP_SECONDARY_Q} --dns-server-ip ${NAMESERVER_IP_SECONDARY_R} ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 60 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_A} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_B} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_C} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_D} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_E} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_F} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_G} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_H} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_I} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_J} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_K} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_L} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_M} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_N} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_O} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_P} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_Q} --dns-server-ip-primary ${NAMESERVER_IP_PRIMARY_R} --dns-server-ip ${NAMESERVER_IP_SECONDARY_A} --dns-server-ip ${NAMESERVER_IP_SECONDARY_B} --dns-server-ip ${NAMESERVER_IP_SECONDARY_C} --dns-server-ip ${NAMESERVER_IP_SECONDARY_D} --dns-server-ip ${NAMESERVER_IP_SECONDARY_E} --dns-server-ip ${NAMESERVER_IP_SECONDARY_F} --dns-server-ip ${NAMESERVER_IP_SECONDARY_G} --dns-server-ip ${NAMESERVER_IP_SECONDARY_H} --dns-server-ip ${NAMESERVER_IP_SECONDARY_I} --dns-server-ip ${NAMESERVER_IP_SECONDARY_J} --dns-server-ip ${NAMESERVER_IP_SECONDARY_K} --dns-server-ip ${NAMESERVER_IP_SECONDARY_L} --dns-server-ip ${NAMESERVER_IP_SECONDARY_M} --dns-server-ip ${NAMESERVER_IP_SECONDARY_N} --dns-server-ip ${NAMESERVER_IP_SECONDARY_O} --dns-server-ip ${NAMESERVER_IP_SECONDARY_P} --dns-server-ip ${NAMESERVER_IP_SECONDARY_Q} --dns-server-ip ${NAMESERVER_IP_SECONDARY_R} ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "found nameserver: ${NAMESERVER_IP_PRIMARY_A}" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "found nameserver: ${NAMESERVER_IP_PRIMARY_R}" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ -z "${output}" ]
  then
    output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_IP_PRIMARY_A} (attempt 1)" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
    if [ "${output}" == "1" ]
    then
      output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_IP_PRIMARY_R} (attempt 1)" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
      if [ "${output}" == "0" ]
      then
        output=`grep "found nameserver: ${NAMESERVER_IP_SECONDARY_A}" ${TMPDIR}/${TEST_NUM}-output.txt`
        if [ ! -z "${output}" ]
        then
          output=`grep "found nameserver: ${NAMESERVER_IP_SECONDARY_R}" ${TMPDIR}/${TEST_NUM}-output.txt`
          if [ -z "${output}" ]
          then
            output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_IP_SECONDARY_A} (attempt 2)" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
            if [ "${output}" == "1" ]
            then
              output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_IP_SECONDARY_R} (attempt 2)" ${TMPDIR}/${TEST_NUM}-output.txt | wc -l | awk '{ print $1 }'`
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
