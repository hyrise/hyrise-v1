#!/usr/bin/python

import os, random, socket, subprocess, cPickle
from python_gdb_common import *

class PyGdbError(Exception):
    def __init__(self, msg):
        assert isinstance(msg, str), "error message must be a str type"
        Exception.__init__(self, msg)
        
class PyGdbStopEvent(Exception):
    def __init__(self, breakpoint):
        assert isinstance(breakpoint, dict), "breakpoint data must be contained by a dict"
        Exception.__init__(self, "Gdb Stop Event")
        self.bp = PyGdbObject(breakpoint)

class GdbHandler:
    def __init__(self, targetFile, gdbPath="/usr/local/bin/gdb", gdbTimeout=1000):
        targetPath = os.path.abspath(targetFile)
        if not os.path.isfile(targetPath):
            raise PyGdbError("Target path does not exist: %s"%(targetPath))
        # We set it as the target executable and load the symbol table
        
        handlerPort = 0
        # Make sure we've got a free port
        for i in xrange(0, 256):
            handlerPort = random.randint(1024, 65535)
            s = socket.socket()
            try:
                s.connect(("127.0.01", handlerPort))
            except socket.error:
                break
            s.close()
            handlerPort = 0
        #
        if handlerPort == 0:
            raise PyGdbError("GdbHandler failed to find a free TCP port!")
        #
        # Ready our server socket
        serverSocket = socket.socket()
        serverSocket.settimeout(gdbTimeout)
        serverSocket.bind(("127.0.0.1", handlerPort))
        serverSocket.listen(1)
        
        # Start the driver with the port number as a parameter
        self._driverPath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "python_gdb_driver.py")
        if not os.path.isfile(self._driverPath):
            raise PyGdbError("Driver path does not exist: '%s'"%(self._driverPath))
        #
        self._driverArgs = gdbPath + " -n -ex \"python handlerPort=" + str(handlerPort) + "\" -ex \"set target-async on\" -ex \"set environment HYRISE_DB_PATH = " + os.getcwd() + "/test/\" -ex \"target exec " + targetPath + "\" -ex \"file " + targetPath + "\" -ex \"r&\" -ex \"source " + self._driverPath + "\""
        self._driverProcess = subprocess.Popen(self._driverArgs, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self._driverSocket, socketAddress = serverSocket.accept()
        serverSocket.close()
    
    #Make sure we clean up the driver process
    def __del__(self):
        if self._driverProcess is not None:
            self._driverProcess.kill()
            self._driverProcess = None
            self._driverSocket.close()
    
    # Make sure we get the correct acknowledgement from the driver
    def _recvAck(self):
        try:
            ackObj = cPickle.loads(self._driverSocket.recv(TransferBufferSize))
            if not ackObj.type == "ack":
                if ackObj.type == "error":
                    raise PyGdbError(ackObj.payload)
                elif ackObj.type == "stopEvent":
                    raise PyGdbStopEvent(ackObj.payload)
                else:
                    raise PyGdbError("Unrecognised driver response code: '%s'"%(ackObj.type))
            else:
                return PyGdbObject(ackObj.payload)
        except EOFError:
            return
    
    # Send a command to the gdb interpreter
    def execute(self, command):
        obj = PyGdbObject()
        obj.type = "execute"
        obj.payload = command
        self._driverSocket.send(cPickle.dumps(obj))
        self._recvAck()
    
    def waitForBreak(self):
        obj = PyGdbObject()
        obj.type = "waitforbreak"
        self._driverSocket.send(cPickle.dumps(obj))


    def addBreakpoint(self, fileName, lineNumber):
        filePath = os.path.abspath(fileName)
        if not os.path.isfile(filePath):
            raise PyGdbError("File does not exist: %s"%(fileName))
        self.execute("break %s:%i" % (filePath, lineNumber))
        
    # Get the current callstack
    def callstack(self):
        obj = PyGdbObject()
        obj.type = "callstack"
        self._driverSocket.send(cPickle.dumps(obj))
        return self._recvAck()
    
    #def wait(self):
    #    if self._driverProcess is None: return
    #    self._driverProcess.wait()
    #    self._driverProcess = None

