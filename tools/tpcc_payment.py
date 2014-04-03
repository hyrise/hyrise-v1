#!/bin/env python2
from client import Connection
c = Connection()
#response = c.stored_procedure("TPCC-Payment", """{ "W_ID": 1, "D_ID": 1, "C_W_ID": 1, "C_D_ID": 1, "H_AMOUNT": 150.0, "C_ID": 1 }""")
#print response
d = Connection()
response = d.stored_procedure("TPCC-Payment", """{ "W_ID": 1, "D_ID": 1, "C_W_ID": 1, "C_D_ID": 1, "H_AMOUNT": 150.0, "C_LAST": "CLName2" }""")
print response
