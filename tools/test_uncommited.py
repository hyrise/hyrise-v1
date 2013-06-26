import re
import subprocess


git_call = "git ls-files --modified --others --exclude-standard"

modules = ["access", "helper", "io", "layouter", "net", "memory", "storage", "taskscheduler"]

result = subprocess.Popen(git_call.split(" "), stdout=subprocess.PIPE)
files =  map(lambda x: x.strip(), result.stdout.read().split("\n"))

tests = set()

for f in files:
	# Check libaries
	for m in modules:
		if re.match("src/lib/%s" % m, f):
			tests.add("build/units_%s" % m)

print(" ".join(tests))