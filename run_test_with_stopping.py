#!/usr/bin/env python

import os
import re
import subprocess
from subprocess import check_output
import datetime
import time
import sys

make_j = 16


#TODO: make paths independent => make project path command line argument!
projectName = "musl"
pathToProject = os.path.abspath("../hash_projects/musl")
clanghashWrapper = os.path.abspath("build/wrappers/clang")
buildInfoFilePath = os.path.abspath("build/muslHashes/buildInfo_%s.info" % projectName)


def checkout(commitID):
    """checkout commit with commitID"""
    os.chdir(pathToProject)
    subprocess.call(["git", "checkout", "-f", "-q", commitID])


def getCommitTime(commitID):
    os.chdir(pathToProject)
    commitTime = check_output(["git", "show", "-s", "--format=%ct", commitID])
    return commitTime.replace('\n', '')


DEBUG = 1
def log(message):
    if (DEBUG == 1):
        print message


################################################################################


log("Starting at %s" % datetime.datetime.now())

os.environ['CC'] = clanghashWrapper
os.environ['PROJECT'] = projectName
os.environ['STOP_IF_SAME_HASH'] = '1'

#reset to latest version
checkout("master")
log("calling make clean and git clean")
subprocess.call(["make", "clean"])
subprocess.call(["git", "clean", "-f", "-q", "-x"])
subprocess.call(["./configure"])
 
commitInfo = {}
commitID = check_output(["git", "log", "-1", "--pretty='%H'"])  # get current commit hash
commitInfo['commit-hash'] = commitID
os.environ['COMMIT_HASH'] = commitID
    
log("calling make -j%d" % make_j)
startTime = time.time()
p = subprocess.Popen(["make", "-j%d" % make_j], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
out, err = p.communicate()
retcode = p.wait()
    
log(">>>>>>>>>>")
log(out)
log(err)
log("<<<<<<<<<<")

commitInfo['build-time'] = int((time.time() - startTime) * 1e9) * make_j # nano * nr of threads
log("finished commit %s at %s" % (commitID, datetime.datetime.now()))

f = open(buildInfoFilePath, 'a')
f.write(repr(commitInfo) + "\n")
f.close()
log("Finished at %s" % datetime.datetime.now())

