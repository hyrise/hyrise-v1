#!/bin/env python2
from client import Connection
c = Connection()
response = c.stored_procedure("TPCC-NewOrder", """{ "W_ID": 1, "D_ID": 1, "C_ID": 1, "O_CARRIER_ID": 5, "OL_DIST_INFO": "lalala", "items": [{"I_ID":1 , "I_W_ID": 1, "quantity": 1},
                                                                                                                                            {"I_ID":2 , "I_W_ID": 1, "quantity": 1},
                                                                                                                                            {"I_ID":7 , "I_W_ID": 1, "quantity": 1},
                                                                                                                                            {"I_ID":11, "I_W_ID": 1, "quantity": 1},
                                                                                                                                            {"I_ID":3 , "I_W_ID": 1, "quantity": 1}]}""")
print response
