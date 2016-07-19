#!/usr/bin/env python

import os
import re
import subprocess
from subprocess import check_output
import datetime
import time
import sys

#TODO: make paths independent => make project path command line argument!
projectName = "musl"
pathToProject = os.path.abspath("../hash_projects/musl")
clanghashWrapper = os.path.abspath("build/wrappers/clang")


def getListOfCommits():
    """get all the commit ids from the project"""
    os.chdir(pathToProject)   
    git_log = check_output(["git", "log"])
    git_log = git_log.split("\n")
    for line in git_log:
        if re.match("commit [0-9a-f]{40}", line):
            yield line[7:47]


def checkout(commitID):
    """checkout commit with commitID"""
    os.chdir(pathToProject)
    subprocess.call(["git", "clean", "-f", "-q", "-x"])
    subprocess.call(["git", "checkout", "-f", "-q", commitID])


DEBUG = 1
def log(message):
    if (DEBUG == 1):
        print message


################################################################################

commitsToHash = 0
if (len(sys.argv) > 1):
    commitsToHash = int(sys.argv[1])

log("Starting at %s" % datetime.datetime.now())

os.environ['CC'] = clanghashWrapper
os.environ['PROJECT'] = projectName

#reset to latest version
checkout("master")

commitCounter = 0
buildTimes = {}
for commitID in getListOfCommits():
    os.environ['COMMIT_HASH'] = commitID
    log ("hashing commit #%d" % commitCounter)
    os.chdir(pathToProject)
    log("calling make clean")
    subprocess.call(["make", "clean"])
    log("checkout")
    checkout(commitID)
    subprocess.call(["./configure"])
    log("calling make -j16")
    startTime = time.time()
    p = subprocess.Popen(["make", "-j16"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    retcode = p.wait()
    buildTimes[commitID] = (time.time() - startTime) * 10e9 # nano
    commitCounter += 1
    log("finished commit %s at %s" % (commitID, datetime.datetime.now()))
    if (commitsToHash > 0 and commitCounter >= commitsToHash):
        break;

#TODO: replace absolute path

f = open("/home/cip/2015/yb90ifym/clang-hash/build/muslHashes/buildTimes_%s.info" % projectName, 'a')
f.write(repr(buildTimes) + "\n")
f.close()
log("Finished at %s" % datetime.datetime.now())
log("Hashed commits: %d" % commitCounter)

