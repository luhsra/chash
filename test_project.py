#!/usr/bin/env python

import os
import re
import subprocess
from subprocess import check_output

pathToProject = os.path.abspath("../hash_projects/musl")

class HashRecord:
    filename = ""
    commitID = 0
    astHash = 0
    objHash = 0
    hashTime = 0
    compileTime = 0    

    def fill(self, filename, commitID, astHash, objHash, hashTime, compileTime):
        self.filename = filename
        self.commitID = commitID
        self.astHash = astHash
        self.objHash = objHash
        self.hashTime = hashTime
        self.compileTime = compileTime

    def toDict(self):
        return {
            "filename" : self.filename,
            "commitID" : self.commitID,
            "astHash" : self.astHash,
            "objHash" : self.objHash,
            "hashTime" : self.hashTime,
            "compileTime" : self.compileTime
        }
   

def getListOfCommits():
    os.chdir(pathToProject)   
    git_log = check_output(["git", "log"])
    git_log = git_log.split("\n")
    
    commits = []
    for line in git_log:
        if re.match("commit [0-9a-f]{40}", line):
            yield line[7:47]



def checkout(commitID):
    #TODO: checkout next commit, run clean for clean build environment
    os.chdir(pathToProject)
    subprocess.call(["git", "checkout", "-f", "-q", commitID])


DEBUG = 1
def log(message):
    if (DEBUG == 1):
        print message


records = []

#TODO: implement iteration over all commits
#TODO: get commit/pull/checkout?
#reset to latest version
checkout("master")

for commitID in getListOfCommits():
    os.chdir(pathToProject)
    log("calling make clean")
    subprocess.call(["make", "clean"])
    log("checkout")
    checkout(commitID)
    log("cp Makefile")
    subprocess.call(["cp", "../Makefile", "Makefile"])
#    make_output = check_output(["make"])
    log("calling make")
    p = subprocess.Popen(["make"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()


    filename = "" #TODO: brauche noch den richtigen filename (vom .c file); momentan das .o file
    astHash = 0
    objHash = 0 #TODO: hash object file
    hashTime = 0 #TODO:  measure hash time in plugin
    compileTime = 0 #TODO: measure compile time

    log ("looping")
    lines = err.split("\n")
    for line in lines:
        if 0 == line.find("dump-ast-file"):
            filename = line.split()[1]
        elif 0 == line.find("top-level-hash"):
            astHash = line.split()[1]
            hashRecord = HashRecord()
            hashRecord.fill(filename, commitID, astHash, objHash, hashTime, compileTime)
            records.append(hashRecord.toDict())
            print hashRecord.toDict()


#for record in records:
#    print record

f = open('hashRecords.txt', 'w') #TODO: ok oder brauche ich w+

f.write("%s" % records)

