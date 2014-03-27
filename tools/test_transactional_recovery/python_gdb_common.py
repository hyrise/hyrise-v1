#http://stackoverflow.com/questions/5021041/are-there-any-gotchas-with-this-python-pattern

TransferBufferSize=65536

class PyGdbObject(dict):
    def __init__(self, d={}):
        dict.__init__(self, d)
        self.__dict__ = self


