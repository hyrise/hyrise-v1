import subprocess
import time
import sys

from client import Connection
import requests

devnull = open('/dev/null', 'w')

def send_query_and_terminate_before_response(port):
    # idea is to send the request but close connection before response was send
    # server has to handle this gracefully
    subprocess.call('curl -X POST --max-time 1 --data-urlencode query@test/json/wait_sleep_2s.json http://localhost:{0}/query/'.format(port).split(' '), stderr=devnull)

def get_num_sockets(pid):
     output = subprocess.check_output('netstat -anp | grep {0} | wc -l'.format(pid), shell=True, stderr=devnull)
     return int(output)

def test_tx(c):
    query = {"operators": {"noop": {"type": "NoOp"}}}

    t1 = c.query(query)["session_context"] # started a new session
    t2 = c.query(query)["session_context"] # started a new session

    assert t1 == t2
    c.commit()
    assert "session_context" in c.query(query) # started a new session
    c.commit()
    #running = c.runningTransactions()
    #assert len(running["contexts"]) == 0
    tid3 = c.query(query)["session_context"] # opens a new transaction
    #running = c.runningTransactions()
    #assert len(running["contexts"]) == 1
    #assert tid3 == running["contexts"][0]["transaction_id"]
    c.rollback() # end transaction by rollback
    #assert not c.runningTransactions()["contexts"] # no contexts should be running
    assert not "session_context" in c.query(query, True) # started a new session, but instantly commit
    #running = c.runningTransactions()
    #assert len(running["contexts"]) == 0

def test_urls(port):
    op = subprocess.check_output(["curl", "-s", "localhost:{0}/urls/".format(port)])
    assert("/query/" in op)
    assert("/urls/" in op)
    assert("/static/" in op)
    assert("/operations/" in op)

def test_perf_flag(port):
    op = subprocess.check_output("curl -X POST -s --data performance=true --data-urlencode query@test/autojson/NoOp.json localhost:{0}/query/".format(port).split(" "))
    assert "performanceData" in op
    op = subprocess.check_output("curl -X POST -s --data performance=false --data-urlencode query@test/autojson/NoOp.json localhost:{0}/query/".format(port).split(" "))
    assert "performanceData" not in op

def test_sockets(port, pid):
    i = get_num_sockets(pid)
    c = Connection(port=port)
    test_tx(c)
    assert i + 1 == get_num_sockets(pid)
    c2 = Connection(port=port)
    test_tx(c2)
    assert i + 2 == get_num_sockets(pid)
    c.close()
    c2.close()


def main():
    p = None
    if len(sys.argv) == 2:
        binary = sys.argv[-1]
        p = subprocess.Popen(binary, stdout=devnull)
        print "Started process", p.pid
    time.sleep(1)
    port = int(open("hyrise_server.port").readlines()[0])
    assert port != 0

    try:
        test_urls(port)
        c = Connection(port=port)

        test_tx(c)
        test_sockets(port, p.pid)
        test_perf_flag(port)
        send_query_and_terminate_before_response(port)
        time.sleep(2)
        send_query_and_terminate_before_response(port)
        subprocess.check_output(["curl", "-s", "localhost:{0}/shutdown/".format(port)])
        c.close()
        print "=====> No errors <====="
    except Exception, e:
        if p:
            print "Error'd out: Terminating..."
            p.terminate()
        raise
    finally:
        if p: # when we initially started a server, we wait for a proper shutdown
            print "Waiting for shutdown..."
            p.wait()

main()
