#!/bin/env python
import requests
import json

class Connection(object):
    def __init__(self, host="localhost", port=5000):
        self._host = host
        self._port = port
        self._context = None
        self._server_base_url = "http://%s:%s/" % (host, port)

    def query(self, json_query, commit=False):
        payload = { "query" : json.dumps(json_query) }
        if (self._context):
            payload["session_context"] = self._context
        if (commit):
            payload["autocommit"] = "true"
        result = requests.post(self._server_base_url + "query/",
                               data = payload)
        json_response = json.loads(result.text)
        self._context = json_response.get("session_context", None)
        return json_response

    def commit(self):
        if not self._context:
            raise Exception("Should not commit without running transaction")
        r = self.query({"operators": {"cm": {"type": "Commit"}}})
        self._context = None
        return r

    def runningTransactions(self):
        return json.loads(requests.get(self._server_base_url + "status/tx").text)
