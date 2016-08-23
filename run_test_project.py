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


def getCommitTime(commitID):
    os.chdir(pathToProject)
    commitTime = check_output(["git", "show", "-s", "--format=%ct", commitID])
    return commitTime.replace('\n', '')



DEBUG = 1
def log(message):
    if (DEBUG == 1):
        print message


################################################################################

commitsToHash = 0
commitsFrom = 0
commitsTo = 0
if len(sys.argv) > 2:
    commitsToFrom = int(sys.argv[1])#TODO: finish this check if it is working
    commitsTo = int(sys.argv[2])
elif len(sys.argv) > 1:
    commitsToHash = int(sys.argv[1])
    

log("Starting at %s" % datetime.datetime.now())

os.environ['CC'] = clanghashWrapper
os.environ['PROJECT'] = projectName

#reset to latest version
checkout("master")

commitCounter = 0
buildTimes = {}
for commitID in getListOfCommits():
    if commitsFrom != 0:
        commitCounter += 1
        continue
    os.environ['COMMIT_HASH'] = commitID
    log ("hashing commit #%d" % commitCounter)
    os.chdir(pathToProject)
    log("calling make clean")
    subprocess.call(["make", "clean"])
    log("checkout")
    checkout(commitID)
    
    # get number of changes
    os.chdir(pathToProject)
    gitshow = subprocess.Popen(["git", "show"], stdout=subprocess.PIPE)
    dstatOut = subprocess.check_output(('diffstat'), stdin=gitshow.stdout)
    gitshow.wait()

    lines = dstatOut.split('\n')
    index = -1
    while lines[index] == '':
        index -= 1
    lastLine = lines[index]
    changedInsertionsDeletions = [int(s) for s in lastLine.split() if s.isdigit()]

    buildTimes[commitID] = {}
    buildTimes[commitID]['commit-time'] = getCommitTime(commitID)
    buildTimes[commitID]['filesChanged'] = changedInsertionsDeletions[0]
    if "insertion" in lastLine: 
        buildTimes[commitID]['insertions'] = changedInsertionsDeletions[1]
        if "deletion" in lastLine:
            buildTimes[commitID]['deletions'] = changedInsertionsDeletions[2]
    elif "deletion" in lastLine:
        buildTimes[commitID]['deletions'] = changedInsertionsDeletions[1]

    try:
        subprocess.call(["./configure"])
    except:
        print "no configure available anymore\n"
        break
    log("calling make -j16")
    startTime = time.time()
    p = subprocess.Popen(["make", "-j16"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    retcode = p.wait()
    buildTimes[commitID]['build-time'] = (time.time() - startTime) * 10e9 # nano #TODO: 10e8?
    commitCounter += 1
    log("finished commit %s at %s" % (commitID, datetime.datetime.now()))
    if (commitsToHash > 0 and commitCounter >= commitsToHash):
        break
    if (commitsTo > 0 and commitCounter > commitsTo):
        break

#TODO: replace absolute path

f = open("/home/cip/2015/yb90ifym/clang-hash/build/muslHashes/commitInfo_%s.info" % projectName, 'a')
f.write(repr(buildTimes) + "\n")
f.close()
log("Finished at %s" % datetime.datetime.now())
log("Hashed commits: %d" % (commitCounter - commitsFrom))

