#!/usr/bin/env python

import os
import sys
from subprocess import check_output

COMMITINFOFILENAME = "/../commitInfo_musl.info"

pathToRecords = "" # gets set to command line parameter
pathToProject = os.path.abspath("../hash_projects/musl")


def getCommitTime(commitID):
    os.chdir(pathToProject)
    commitTime = check_output(["git", "show", "-s", "--format=%ct", commitID])
    return commitTime


################################################################################


# main:
if (len(sys.argv) > 1):
    pathToRecords = sys.argv[1]
    pathToCommitInfoFile = os.path.abspath(pathToRecords + COMMITINFOFILENAME)

    commitInfo = {}
    with open(pathToCommitInfoFile, 'r') as commitInfoFile:
        commitInfo = eval(commitInfoFile.read())
        for commitID in commitInfo:
            commitTime = getCommitTime(commitID).replace('\n', '')
            commitInfo[commitID]['commit-time'] = commitTime

    f = open(pathToCommitInfoFile, 'w')
    f.write(repr(commitInfo) + "\n")
    f.close()

else:
    print "Missing path to record files.\nUsage:\n\t%s pathToRecords" % sys.argv[0]

