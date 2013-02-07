import urllib
import urllib2
import json
import sys
import commands
import os

HYRISE_SPEED_CENTER = "http://192.168.31.39:8000/"

def addInformation(data):
    for result in data:
        result["commitid"]    = commands.getstatusoutput("git rev-list --max-count=1 HEAD")[1]
	result["branch"]      = "master"
	result["environment"] = commands.getstatusoutput("hostname -s")[1]
        result["executable"]  = "hyrise"
    return {"json" : json.dumps(data)} 

def add(data):
    response = "None"
    try:
      f = urllib2.urlopen(
      HYRISE_SPEED_CENTER + 'result/add/json/', urllib.urlencode(data))
    except urllib2.HTTPError as e:
           print str(e)
           print e.read()
           return
    response = f.read()
    f.close()
    print " (%s) response: %s\n" % (HYRISE_SPEED_CENTER, response)

if __name__ == "__main__":
    f = open(sys.argv[1]).read()
    f = f.replace(',}','}')
    data = [result for result in json.loads(f) if result.has_key(u"result_value")]
    add(addInformation(data))
    
