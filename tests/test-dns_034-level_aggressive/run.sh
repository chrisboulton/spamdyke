# This test checks the aggressive DNS query behavior: all primary
# servers queried simultaneously until max-retries-primary is reached,
# then all servers queried simultaneously.

export TCPREMOTEIP=11.22.33.44
export NAMESERVER_PRIMARY_IP_1=127.0.0.1:52
export NAMESERVER_PRIMARY_IP_2=127.0.0.1:50
export NAMESERVER_SECONDARY_IP_1=127.0.0.1:51
export NAMESERVER_SECONDARY_IP_2=127.0.0.1:49

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-timeout-secs 10 --dns-server-ip-primary ${NAMESERVER_PRIMARY_IP_1} --dns-server-ip-primary ${NAMESERVER_PRIMARY_IP_2} --dns-server-ip ${NAMESERVER_SECONDARY_IP_1} --dns-server-ip ${NAMESERVER_SECONDARY_IP_2} --dns-level aggressive ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-timeout-secs 10 --dns-server-ip-primary ${NAMESERVER_PRIMARY_IP_1} --dns-server-ip-primary ${NAMESERVER_PRIMARY_IP_2} --dns-server-ip ${NAMESERVER_SECONDARY_IP_1} --dns-server-ip ${NAMESERVER_SECONDARY_IP_2} --dns-level aggressive ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_PRIMARY_IP_1} (attempt 1)" ${TMPDIR}/${TEST_NUM}-output.txt`
if [ ! -z "${output}" ]
then
  output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_PRIMARY_IP_2} (attempt 1)" ${TMPDIR}/${TEST_NUM}-output.txt`
  if [ ! -z "${output}" ]
  then
    output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_PRIMARY_IP_1} (attempt 2)" ${TMPDIR}/${TEST_NUM}-output.txt`
    if [ ! -z "${output}" ]
    then
      output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_PRIMARY_IP_2} (attempt 2)" ${TMPDIR}/${TEST_NUM}-output.txt`
      if [ ! -z "${output}" ]
      then
        output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_PRIMARY_IP_1} (attempt 3)" ${TMPDIR}/${TEST_NUM}-output.txt`
        if [ ! -z "${output}" ]
        then
          output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_PRIMARY_IP_2} (attempt 3)" ${TMPDIR}/${TEST_NUM}-output.txt`
          if [ ! -z "${output}" ]
          then
            output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_PRIMARY_IP_1} (attempt 4)" ${TMPDIR}/${TEST_NUM}-output.txt`
            if [ -z "${output}" ]
            then
              output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_PRIMARY_IP_2} (attempt 4)" ${TMPDIR}/${TEST_NUM}-output.txt`
              if [ -z "${output}" ]
              then
                output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP_1} (attempt 1)" ${TMPDIR}/${TEST_NUM}-output.txt`
                if [ -z "${output}" ]
                then
                  output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP_2} (attempt 1)" ${TMPDIR}/${TEST_NUM}-output.txt`
                  if [ -z "${output}" ]
                  then
                    output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP_1} (attempt 2)" ${TMPDIR}/${TEST_NUM}-output.txt`
                    if [ ! -z "${output}" ]
                    then
                      output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP_2} (attempt 2)" ${TMPDIR}/${TEST_NUM}-output.txt`
                      if [ ! -z "${output}" ]
                      then
                        output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP_1} (attempt 3)" ${TMPDIR}/${TEST_NUM}-output.txt`
                        if [ ! -z "${output}" ]
                        then
                          output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP_2} (attempt 3)" ${TMPDIR}/${TEST_NUM}-output.txt`
                          if [ ! -z "${output}" ]
                          then
                            output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP_1} (attempt 4)" ${TMPDIR}/${TEST_NUM}-output.txt`
                            if [ -z "${output}" ]
                            then
                              output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP_2} (attempt 4)" ${TMPDIR}/${TEST_NUM}-output.txt`
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
