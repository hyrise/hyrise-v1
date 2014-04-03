#!/bin/env python2
from client import Connection
c = Connection()
response = c.stored_procedure("TPCC-OrderStatus", """{ "W_ID": 1, "D_ID": 1, "C_ID": 1}""")
print response

d = Connection()
response = d.stored_procedure("TPCC-OrderStatus", """{ "W_ID": 1, "D_ID": 1, "C_LAST": "CLName1"}""")
print response
