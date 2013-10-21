import subprocess
import time
import signal
import sys

from client import Connection
import requests

def main():
    devnull = open('/dev/null', 'w')
    assert(len(sys.argv) == 2)
    binary = sys.argv[-1]
    p = subprocess.Popen(binary, stdout=devnull)
    time.sleep(3)
    port = int(open("hyrise_server.port").readlines()[0])
    assert(port != 0)
    op = subprocess.check_output(["curl", "-s", "localhost:{0}/urls/".format(port)])
    try:
        assert("/query/" in op)
        assert("/urls/" in op)
        assert("/static/" in op)
        assert("/operations/" in op)

        c = Connection(port=port)
        query = {"operators": {"noop": {"type": "NoOp"}}}

        t1 = c.query(query)["session_context"] # started a new session
        t2 = c.query(query)["session_context"] # started a new session

        assert t1 == t2
        c.commit()
        assert "session_context" in c.query(query) # started a new session
        c.commit()
        running = c.runningTransactions()
        assert len(running["contexts"]) == 0
        tid3 = c.query(query)["session_context"] # opens a new transaction
        running = c.runningTransactions()
        assert len(running["contexts"]) == 1
        assert tid3 == running["contexts"][0]["transaction_id"]
        c.rollback() # end transaction by rollback
        assert not c.runningTransactions()["contexts"] # no contexts should be running
        assert not "session_context" in c.query(query, True) # started a new session, but instantly commit
        running = c.runningTransactions()
        assert len(running["contexts"]) == 0
        print "=====> No errors <====="
    finally:
        p.send_signal(signal.SIGINT)
        p.wait()

main()
