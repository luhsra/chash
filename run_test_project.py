#!/usr/bin/env python

import os
import re
import subprocess
from subprocess import check_output
import datetime

#TODO: die pfade alle unabhaengig machen; mkdir outputPath im skript machen
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



log("Starting at %s" % datetime.datetime.now())

records = []
os.environ['CC'] = clanghashWrapper

#reset to latest version
checkout("master")

commitCounter = 0
for commitID in getListOfCommits():
    log ("hashing commit #%d" % commitCounter)
    records = [] #TODO: ist das ok? sollte am schluss am besten eine einzige map sein => am besten halt einfach die oben/close klammern per hand aussenrum machen
    os.chdir(pathToProject)
    log("calling make clean")
    subprocess.call(["make", "clean"])
    log("checkout")
    checkout(commitID)
    subprocess.call(["./configure"])
    log("calling make -j16")
    p = subprocess.Popen(["make", "-j16"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    retcode = p.wait()
    
    commitCounter += 1
    log("finished commit %s at %s" % (commitID, datetime.datetime.now()))

log("Finished at %s" % datetime.datetime.now())
log("Total commits: %d" % (commitCounter + 1))

