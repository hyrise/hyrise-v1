import subprocess
import time
import signal

def main():
    p = subprocess.Popen("./build/hyrise_server")
    time.sleep(3)
    port = int(open("hyrise_server.port").readlines()[0])
    assert(port != 0)
    op = subprocess.check_output(["curl", "-s", "localhost:{0}/urls/".format(port)])
    try:
        assert("/query/" in op)
        assert("/urls/" in op)
        assert("/static/" in op)
        print "=====> No errors <====="
    finally:
        p.send_signal(signal.SIGINT)
        p.wait()
        
main()
