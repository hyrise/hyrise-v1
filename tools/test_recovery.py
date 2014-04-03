import subprocess
import time
import sys
import os
import json

from client import Connection
import requests

devnull = open('/dev/null', 'w')

def main():
    subprocess.call(os.path.expandvars("rm /mnt/pmfs/$USER/hyrisedata/mcidx__test_recovery_revenue__* /mnt/pmfs/$USER/hyrisedata/test_recovery_revenue__*"), stderr=devnull, shell=True)

    p = None
    if len(sys.argv) == 2:
        binary = sys.argv[-1]
        myenv = os.environ.copy()
        myenv["HYRISE_DB_PATH"] = os.getcwd() + "/test/"
        p = subprocess.Popen(binary, stdout=devnull, env=myenv)
        print "Started process", p.pid

    time.sleep(1)
    port = int(open("hyrise_server.port").readlines()[0])
    assert port != 0

    try:
        op = subprocess.check_output("curl -X POST -s --data-urlencode query@./test/json/load_revenue.json localhost:{0}/query/".format(port).split(" "))
        p.kill()
        print "SIGKILLed process", p.pid

        p = subprocess.Popen([binary, "--recover"], stdout=devnull, env=myenv)
        print "Started process", p.pid
        time.sleep(1)
        op = subprocess.check_output("curl -X POST -s --data-urlencode query@./test/json/compound_scan_revenue.json localhost:{0}/query/".format(port).split(" "))
        j = json.loads(op)
        if not "rows" in j:
            raise Exception("Unexpected JSON", op)
        if str(j["rows"]) != "[[2009, 3, 3000], [2009, 5, 3000]]":
            raise Exception("Wrong result", op)
        subprocess.check_output(["curl", "-s", "localhost:{0}/shutdown/".format(port)])
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
