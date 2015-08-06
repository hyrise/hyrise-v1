#!/bin/env python2
from client import Connection
c = Connection()
response = c.stored_procedure("TPCC-Delivery", """{ "W_ID": 1, "D_ID": 1, "O_CARRIER_ID": 1}""")
print response
