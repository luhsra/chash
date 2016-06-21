#!/usr/bin/env python

import os
import re
import subprocess
from subprocess import check_output
import datetime

pathToProject = os.path.abspath("../hash_projects/musl")
hashObjectfile = os.path.abspath("wrappers/hash-objectfile")
outputFilename = "hashRecords.txt";

class HashRecord:
    filename = ""
    commitID = 0
    astHash = 0
    objHash = 0
    hashTime = 0
    compileTime = 0
    processedBytes = 0

    def fill(self, filename, commitID, astHash, objHash, hashTime, compileTime, processedBytes):
        self.filename = filename
        self.commitID = commitID
        self.astHash = astHash
        self.objHash = objHash
        self.hashTime = hashTime
        self.compileTime = compileTime
        self.processedBytes = processedBytes

    def toDict(self):
        return {
            "filename" : self.filename,
            "commitID" : self.commitID,
            "astHash" : self.astHash,
            "objHash" : self.objHash,
            "hashTime" : self.hashTime,
            "compileTime" : self.compileTime,
            "processedBytes" : self.processedBytes
        }
   

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
#    subprocess.call(["git", "clean", "-f", "-q", "-x"]) #TODO: geht iwie nicht richtig...
# => bloede idee, removed auch die sachen vom configure...
# => vllt. auch configure noch extra aufrufen?
    subprocess.call(["git", "checkout", "-f", "-q", commitID])


def getSourceFilename(objectfile):
    """removes "obj/" from the path and replaces .o with .c"""
    filename = objectfile[4:]
    filename = filename[:-1] + 'c'
    return filename


DEBUG = 1
def log(message):
    if (DEBUG == 1):
        print message



log("Starting at %s" % datetime.datetime.now())

records = []

#reset to latest version
checkout("master")

try:
    os.remove(outputFilename) #TODO: -> already gets removed by git clean
except OSError:
    pass

f = open(outputFilename, 'a')
commitCounter = 0
for commitID in getListOfCommits():
    log ("hashing commit #%d" % commitCounter)
    records = [] #TODO: ist das ok? sollte am schluss am besten eine einzige map sein => am besten halt einfach die oben/close klammern per hand aussenrum machen
    os.chdir(pathToProject)
    log("calling make clean")
    subprocess.call(["make", "clean"])
    log("checkout")
    checkout(commitID)
    log("cp Makefile")
    subprocess.call(["cp", "../Makefile", "Makefile"])
    log("calling make")
    p = subprocess.Popen(["make"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()


    filename = ""
    astHash = 0
    objHash = 0
    hashTime = 0
    compileTime = 0 #TODO: measure compile time
    processedBytes = 0

    log ("looping")
    lines = err.split("\n")
    for line in lines:
        log(line)
#    log(lines) #TODO: write this to a file and break after first commit for error search
    for line in lines:
        if 0 == line.find("dump-ast-file"):
            objFilename = line.split()[1];
            filename = getSourceFilename(objFilename)
            log(filename)
            objHash = check_output([hashObjectfile, objFilename])
        elif 0 == line.find("top-level-hash"):
            astHash = line.split()[1]
        elif 0 == line.find("processed bytes:"):
            processedBytes = line.split()[1]
        elif 0 == line.find("elapsed time (s):"):
            hashTime = line.split()[1]
            hashRecord = HashRecord()
            hashRecord.fill(filename, commitID, astHash, objHash, hashTime, compileTime, processedBytes)
            records.append(hashRecord.toDict())
            
    for record in records:
        f.write("%s\n" % record)
        log(record)
    commitCounter += 1
    log("finished commit %s" % commitID)

log("Finished at %s" % datetime.datetime.now())
log("Total commits: &d" % (commitCounter + 1))

