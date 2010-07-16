# This test checks the number of DNS retries: 2 to the primary, 1 more
# to both.

export TCPREMOTEIP=11.22.33.44
export NAMESERVER_PRIMARY_IP=127.0.0.1:52
export NAMESERVER_SECONDARY_IP=127.0.0.1:51

cp input.txt ${TMPDIR}/${TEST_NUM}-input.txt
echo "${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-timeout-secs 10 --dns-server-ip-primary ${NAMESERVER_PRIMARY_IP} --dns-server-ip ${NAMESERVER_SECONDARY_IP} --dns-max-retries-primary 2 ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1"
${SENDRECV_PATH} -t 30 -r 221 -- ${SPAMDYKE_PATH} -lexcessive --log-target stderr --dns-timeout-secs 10 --dns-server-ip-primary ${NAMESERVER_PRIMARY_IP} --dns-server-ip ${NAMESERVER_SECONDARY_IP} --dns-max-retries-primary 2 ${QMAIL_CMDLINE} < ${TMPDIR}/${TEST_NUM}-input.txt > ${TMPDIR}/${TEST_NUM}-output.txt 2>&1

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
      if [ -z "${output}" ]
      then
        output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP} (attempt 2)" ${TMPDIR}/${TEST_NUM}-output.txt`
        if [ -z "${output}" ]
        then
          output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP} (attempt 3)" ${TMPDIR}/${TEST_NUM}-output.txt`
          if [ ! -z "${output}" ]
          then
            output=`grep "for 44.33.22.11.in-addr.arpa(PTR) to DNS server ${NAMESERVER_SECONDARY_IP} (attempt 4)" ${TMPDIR}/${TEST_NUM}-output.txt`
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
