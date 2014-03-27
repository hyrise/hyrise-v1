#!/bin/env python2
from client import Connection
c = Connection()
response = c.stored_procedure("TPCC-StockLevel", """{ "W_ID": 1, "D_ID": 1, "threshold": 35 }""")
print response
