# These tests aim to cover all possible transactional failure scenarios for Hyrise on NVRAM:
#
# fail_after_insert:            Inserts a row but does not commit, then fails.
#                               Row shall not be visible after recovery.
#
# fail_after_update:            Updates a row but does not commit, then fails.
#                               Previous version shall be visible, new version not.
#
# fail_during_insert_commit:    Inserts a row, starts to commit, but fails before
#                               last CID is increased. Row shall be invisible after recovery.
#
# fail_during_update_commit:    Updates a row, starts to commit, but fails before
#                               last CID is increased. Old row shall be visible, new row invisible after
#                               recovery.
#
# fail_after_insert_commit:     Inserts a row, commits, then fails
#                               last CID is increased. Row shall be visible after recovery.
#
# fail_after_update_commit:     Updates a row, commits, then fails
#                               last CID is increased. Old row shall be invisible, new row visible after
#                               recovery.

# SIMPLE relative imports.
import sys
import os
CWD = os.path.abspath(os.path.dirname(__file__))
sys.path.append(CWD)

import subprocess
import time
import json
import python_gdb as pygdb
import python_gdb_common

p = 0
port = 0
pid = 0
devnull = open('/dev/null', 'w')

expect_unchanged = [[2009,1,2000],[2009,2,2500],[2009,3,3000],[2009,4,4000],[2010,1,2400],[2010,2,2800],[2010,3,3200],[2010,4,3600]]
expect_inserted  = [[2009,1,2000],[2009,2,2500],[2009,3,3000],[2009,4,4000],[2010,1,2400],[2010,2,2800],[2010,3,3200],[2010,4,3600],[2013,1,2000],[2013,2,2500],[2013,3,3000],[2013,4,4000]]
expect_updated   = [[2009,1,2000],[2009,2,2500],[2009,3,3000],[2009,4,4000],[2010,1,2400],[2010,2,2800],[2010,4,3600],[2010,3,10000]]

def cleanup():
    subprocess.call(os.path.expandvars("rm /mnt/pmfs/$USER/hyrisedata/* /mnt/pmfs/$USER/*"), shell=True, stderr=devnull)

def start_hyrise():
    if len(sys.argv) == 2:
        binary = sys.argv[-1]
        myenv = os.environ.copy()
        myenv["HYRISE_DB_PATH"] = os.getcwd() + "/test/"
        p = subprocess.Popen([binary, '--recover'], env=myenv, stdout=devnull, stderr=devnull)
        time.sleep(1)
    else:
        raise Exception("No server binary specified")

def kill_hyrise():
    subprocess.check_output("kill -9 {0}".format(pid).split(" "))

def get_instance():
    global port, pid
    port = int(open("hyrise_server.port").readlines()[0])
    pid = int(open("hyrise_server.pid").readlines()[0])
    assert port != 0
    assert pid != 0

def load():
    send_json("load_revenue")

def validate(expected):
    op = send_json("validate_revenue")
    j = json.loads(op)
    if not "rows" in j:
        raise Exception("Unexpected JSON", op)
    if j["rows"] != expected:
        print
        print "Got:      " + str(j["rows"]) + "\nExpected: " + str(expected)
        raise Exception("Wrong result")

def teardown():
    subprocess.check_output(["curl", "-s", "localhost:{0}/shutdown/".format(port)])

def send_json(json_file, async=False):
    if async:
        subprocess.Popen(("curl -X POST -s --data-urlencode query@./tools/test_transactional_recovery/" + json_file + ".json localhost:{0}/query/".format(port)).split(" "))
    else:
        return subprocess.check_output(("curl -X POST -s --data-urlencode query@./tools/test_transactional_recovery/" + json_file + ".json localhost:{0}/query/".format(port)).split(" "))

def test(expected):
    def _test(func):
        def __test():
            cleanup()
            start_hyrise()
            get_instance()
            load()
            func()
            kill_hyrise()
            start_hyrise()
            validate(expected)
            teardown()
            sys.stdout.write('.')
            sys.stdout.flush()
        return __test
    return _test

def gdbtest(killpoint, expected, **kwargs):
    def _gdbtest(func):
        def __gdbtest():
            cleanup()
            handler = pygdb.GdbHandler(sys.argv[-1])

            handler.execute("b " + killpoint)
            handler.waitForBreak()
            time.sleep(2)
            get_instance()
            load()
            try:
                func()
                handler._recvAck()
            except pygdb.PyGdbStopEvent, e:
                kill_hyrise()
                handler.execute("set confirm off")
                handler.execute("quit")

            start_hyrise()
            if kwargs.get("commit_after_restart", False):
                send_json("commit")

            validate(expected)
            teardown()
            sys.stdout.write('.')
            sys.stdout.flush()
        return __gdbtest
    return _gdbtest

@test(expect_unchanged)
def fail_after_insert():
    send_json("insert_revenue")

@test(expect_unchanged)
def fail_after_update():
    send_json("update_revenue")

@gdbtest("hyrise::tx::TransactionManager::commit", expect_unchanged)
def fail_during_insert_commit():
    send_json("insert_revenue_commit", True)

@gdbtest("hyrise::tx::TransactionManager::endTransaction", expect_unchanged)
def fail_after_insert_commit():
    send_json("insert_revenue_commit", True)

@gdbtest("hyrise::tx::TransactionManager::commit", expect_unchanged, commit_after_restart=True)
def fail_during_insert_commit_then_commit_again():
    send_json("insert_revenue_commit", True)

@gdbtest("hyrise::tx::TransactionManager::commit", expect_unchanged)
def fail_during_update_commit():
    send_json("update_revenue_commit", True)

@gdbtest("hyrise::tx::TransactionManager::endTransaction", expect_unchanged)
def fail_after_update_commit():
    send_json("update_revenue_commit", True)

@gdbtest("hyrise::tx::TransactionManager::commit", expect_unchanged, commit_after_restart=True)
def fail_during_update_commit_then_commit_again():
    send_json("update_revenue_commit", True)

if __name__ == "__main__":
    sys.stdout.write("Testing recovery (this will take a while): ")
    sys.stdout.flush()

    fail_after_insert()
    fail_after_update()
    fail_during_insert_commit()
    fail_after_insert_commit()
    fail_during_insert_commit_then_commit_again()
    fail_during_update_commit()
    fail_after_update_commit()
    fail_during_update_commit_then_commit_again()

    print " - passed"