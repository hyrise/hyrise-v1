#!/bin/env python
import requests
import json

class Connection(object):
    def __init__(self, host="localhost", port=5000):
        self._host = host
        self._port = port
        self._context = None
        self._session = requests.Session()
        self._server_base_url = "http://%s:%s/" % (host, port)

    def close(self):
        self._session.close()

    def query(self, json_query, commit=False):
        result = self.query_raw(json.dumps(json_query), self._context, commit)
        json_response = json.loads(result)
        self._context = json_response.get("session_context", None)
        return json_response

    def query_raw(self, query, context, commit=False):
        payload = { "query" : query }
        if context:
            payload["session_context"] = context
        if commit:
            payload["autocommit"] = "true"
        result = self._session.post(self._server_base_url + "query/",
                               data = payload, headers={'Connection': 'Keep-Alive'})
        return result.text

    def commit(self):
        if not self._context:
            raise Exception("Should not commit without running context")
        r = self.query({"operators": {"cm": {"type": "Commit"}}})
        return r

    def rollback(self):
        if not self._context:
            raise Exception("Should not rollback without running context")
        r = self.query({"operators": {"rb": {"type": "Rollback"}}})
        return r

    def runningTransactions(self):
        return json.loads(self._session.get(self._server_base_url + "status/tx").text)
