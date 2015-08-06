import gdb, socket, cPickle, os, time
CWD = os.path.abspath(os.path.dirname(__file__))
sys.path.append(CWD)
from python_gdb_common import *
#import gdb_printers

class GdbDriver:
    
    def __init__(self):
        # We register some handlers for events
        gdb.events.stop.connect (self.stopEvent)
        # gdb.events.exited.connect(self.exitEvent)
        self.gdbEvent = None
        self.abortStatus = None
        self.gdbFrames = []
        self.waiting = False
    
    # When this event gets set, we need to signal back to the driver immediately to proess the event data
    def setGdbEvent(self, eventType, eventData):
        assert isinstance(eventData, dict), "error gdb event data must be contained by a dict"
        if self.gdbEvent is not None:
            self.abortStatus = "GdbDriver error, gdb event structure is being set multiple times without reset"
            raise Exception(self.abortStatus)
        self.gdbEvent = PyGdbObject()
        self.gdbEvent.type = eventType
        self.gdbEvent.payload = eventData
    
    def connect(self):
        global handlerPort
        self.socket = socket.socket()
        self.socket.connect(("127.0.0.1", handlerPort))
    
    def stopEvent(self, event):
        eventData = {}
        bp = event.breakpoint
        bpAttributes = ["number", "location"]
        eventData ["type"] = type(bp).__name__
        for attr in bpAttributes:
            if hasattr(bp, attr):
                eventData [attr] = getattr(bp, attr)
        #
        if event.inferior_thread is not None:
            eventData ["threadNumber"] = event.inferior_thread.num
        else:
            eventData ["threadNumber"] = None
        #
        self.setGdbEvent("stopEvent", eventData)
        #
        if self.waiting:
            self.waiting = False
            self.socket.send(cPickle.dumps(self.gdbEvent))
            self.session()
        return True
    
    def exitEvent(self, event):
        return True
    
    def runInferior(self):
        runCommand="run"
        while True:
            gdb.execute(runCommand)
            runCommand = "continue"
            print "** continue **"
    
    def sendAck(self, payload={}):
        assert isinstance(payload, dict), "ack payloads must be dictionaries"
        self.socket.send(cPickle.dumps(PyGdbObject({"type":"ack", "payload":payload})))
    
    def sendError(self, payload):
        assert isinstance(payload, str), "error payloads must be strings"
        self.socket.send(cPickle.dumps(PyGdbObject({"type":"error", "payload":payload})))
    
    def execute(self, command):
        self.gdbEvent = None
        self.abortStatus = None
        
        try:
            print command
            gdb.execute(command, True)
        except gdb.error as e:
            self.sendError("%s" % (e))
            return
        
        #print self.abortStatus, self.gdbEvent
        
        if self.abortStatus is not None:
            self.sendError("%s" % (self.abortStatus))
        elif self.gdbEvent is not None:
            #import rpdb2; rpdb2.start_embedded_debugger("stepaside")
            self.socket.send(cPickle.dumps(self.gdbEvent))
        else:
            self.sendAck()
    
    def callstack(self):
        self.gdbFrames = []
        frame = gdb.selected_frame()
        #import rpdb2; rpdb2.start_embedded_debugger("stepaside")
        pyGdbFrames = []
        while frame is not None:
            fullName = frame.function().symtab.fullname()
            lineNumber = frame.find_sal().line
            functionName = frame.function().name
            pyGdbFrames.append(PyGdbObject({
                                            "fullName":fullName,
                                            "lineNumber":lineNumber,
                                            "functionName":functionName
                                           }))
            self.gdbFrames.append(frame)
            frame = frame.older()
        self.sendAck({"frames":pyGdbFrames})
    
    def session(self):
        while True:
            cmdObj = cPickle.loads(self.socket.recv(TransferBufferSize))
            if cmdObj.type == "execute":
                self.execute(cmdObj.payload)
            elif cmdObj.type == "callstack":
                self.callstack()
            elif cmdObj.type == "waitforbreak":
                self.waiting = True
                return
            else:
                print "Unknown session command: %s" % (cmdObj.type)
            

driver = GdbDriver()
driver.connect()
driver.session()
