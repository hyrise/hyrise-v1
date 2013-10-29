#!/bin/env python2
from client import Connection
c = Connection()
response = c.stored_procedure("TPCC-NewOrder", """{ "W_ID": 1, "D_ID": 1, "C_ID": 1, "O_CARRIER_ID": 5, "OL_DIST_INFO": "lalala", "items": [{1, 1, 5}] }""")
print response
