# This test checks the number of DNS retries: 3 to the primary, 3 more
# to both.

export TCPREMOTEIP=11.22.33.44

echo "44.33.22.11.in-addr.arpa PTR IGNORE" > ${TMPDIR}/${TEST_NUM}-dns_conf.txt

NAMESERVER_PRIMARY_IP=127.0.0.1:`${DNSDUMMY_PATH} -t 60 -f ${TMPDIR}/${TEST_NUM}-dns_conf.txt`
NAMESERVER_SECONDARY_IP=127.0.0.1:`${DNSDUMMY_PATH} -t 60 -f ${TMPDIR}/${TEST_NUM}-dns_conf.txt`

echo log-level=excessive >> ${TMPDIR}/${TEST_NUM}-config.txt
echo log-target=stderr >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-timeout-secs=10 >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-server-ip-primary=${NAMESERVER_PRIMARY_IP} >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-server-ip=${NAMESERVER_SECONDARY_IP} >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-max-retries-primary=3 >> ${TMPDIR}/${TEST_NUM}-config.txt
echo dns-max-retries-total=6 >> ${TMPDIR}/${TEST_NUM}-config.txt

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -f ${TMPDIR}/${TEST_NUM}-config.txt ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_PRIMARY_IP} (attempt 1)" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_PRIMARY_IP} (attempt 2)" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_PRIMARY_IP} (attempt 3)" ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ ! -z "${output}" ]
    then
      output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_PRIMARY_IP} (attempt 4)" ${TMPDIR}/${TEST_NUM}-output.txt`
      if [ ! -z "${output}" ]
      then
        output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_PRIMARY_IP} (attempt 5)" ${TMPDIR}/${TEST_NUM}-output.txt`
        if [ ! -z "${output}" ]
        then
          output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_PRIMARY_IP} (attempt 6)" ${TMPDIR}/${TEST_NUM}-output.txt`
          if [ ! -z "${output}" ]
          then
            output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_PRIMARY_IP} (attempt 7)" ${TMPDIR}/${TEST_NUM}-output.txt`
            if [ -z "${output}" ]
            then
              output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP} (attempt 2)" ${TMPDIR}/${TEST_NUM}-output.txt`
              if [ -z "${output}" ]
              then
                output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP} (attempt 3)" ${TMPDIR}/${TEST_NUM}-output.txt`
                if [ -z "${output}" ]
                then
                  output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP} (attempt 4)" ${TMPDIR}/${TEST_NUM}-output.txt`
                  if [ ! -z "${output}" ]
                  then
                    output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP} (attempt 5)" ${TMPDIR}/${TEST_NUM}-output.txt`
                    if [ ! -z "${output}" ]
                    then
                      output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP} (attempt 6)" ${TMPDIR}/${TEST_NUM}-output.txt`
                      if [ ! -z "${output}" ]
                      then
                        output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP} (attempt 7)" ${TMPDIR}/${TEST_NUM}-output.txt`
                        if [ -z "${output}" ]
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
