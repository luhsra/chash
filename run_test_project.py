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
commitInfoFilePath = os.path.abspath("build/muslHashes/commitInfo_%s.info" % projectName)

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
sliceID = '-1' 
if len(sys.argv) > 2:
    commitsFrom = int(sys.argv[1]) - 1
    commitsTo = int(sys.argv[2])
    assert commitsFrom >= 0
    assert commitsTo >= commitsFrom
    commitsToHash = commitsTo - commitsFrom
elif len(sys.argv) > 1:
    commitsToHash = int(sys.argv[1])
    assert commitsToHash > 0

for i in range(0, len(sys.argv)):
    if sys.argv[i] == '-id':
       sliceID = sys.argv[i] 
 

log("Starting at %s" % datetime.datetime.now())

os.environ['CC'] = clanghashWrapper
os.environ['PROJECT'] = projectName
os.environ['SLICE_ID'] = sliceID

#reset to latest version
checkout("master")

commitCounter = 0
commitInfo = {}
for commitID in getListOfCommits():
    if commitsFrom > commitCounter:
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

    commitInfo[commitID] = {}
    commitInfo[commitID]['commit-time'] = int(getCommitTime(commitID))
    commitInfo[commitID]['files-changed'] = changedInsertionsDeletions[0]
    if "insertion" in lastLine: 
        commitInfo[commitID]['insertions'] = changedInsertionsDeletions[1]
        if "deletion" in lastLine:
            commitInfo[commitID]['deletions'] = changedInsertionsDeletions[2]
    elif "deletion" in lastLine:
        commitInfo[commitID]['deletions'] = changedInsertionsDeletions[1]

    try:
        configureStartTime = time.time() 
        subprocess.call(["./configure"])
        commitInfo[commitID]['configure-time'] = int((time.time() - configureStartTime) * 1e9) # ns
    except OSError:
        print "no configure available anymore\n"
        break
    log("calling make -j%d" % make_j)
    startTime = time.time()
    p = subprocess.Popen(["make", "-j%d" % make_j], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    retcode = p.wait()
    commitInfo[commitID]['build-time'] = int((time.time() - startTime) * 1e9) * make_j # nano * nr of threads
    commitCounter += 1
    log("finished commit %s at %s" % (commitID, datetime.datetime.now()))
    if (commitsToHash > 0 and commitCounter >= (commitsToHash + commitsFrom)):
        break

f = open(commitInfoFilePath, 'a')
f.write(repr(commitInfo) + "\n")
f.close()
log("Finished at %s" % datetime.datetime.now())
log("Hashed commits: %d" % (commitCounter - commitsFrom))

