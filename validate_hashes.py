#!/usr/bin/env python

import fnmatch
import os
import sys
from operator import itemgetter


def getListOfFiles(directory):
    for root, dirnames, filenames in os.walk(directory):
        for filename in fnmatch.filter(filenames, '*.info'):
            yield os.path.join(root, filename)


def printHashInfo(message, prevRecord, record, isError=True):
    print "%s: file %s, commits %s to %s : %s" % ("ERROR" if isError else "INFO", record['filename'], prevRecord['commit-hash'], record['commit-hash'], message)

 
errorCount = 0
astDifferObjSameCount = 0
missingCount = 0    

def validateHashes(records):
    global errorCount, astDifferObjSameCount, missingCount
    #TODO: this method assumes that all records are from the same object file

    iterRecords = iter(records)
    prevRecord = next(iterRecords)
    filename = prevRecord['filename']
    if 'ast-hash' not in prevRecord.keys():
        print "MISSING: no ast-hash in records for file " + filename
        missingCount += 1
        return
    for record in iterRecords:
        if prevRecord['ast-hash'] == record['ast-hash']:
            if prevRecord['object-hash'] != record['object-hash']:
                printHashInfo("ast hashes same, object hashes differ", prevRecord, record)
                errorCount += 1
        if prevRecord['object-hash'] != record['object-hash']:
            if prevRecord['ast-hash'] == record['ast-hash']:
                printHashInfo("object hashes differ, ast hashes same", prevRecord, record)
                errorCount += 1
        elif prevRecord['ast-hash'] != record['ast-hash']:
            printHashInfo("ast hashes differ, object hashes same", prevRecord, record, False)
            astDifferObjSameCount += 1

        prevRecord = record


################################################################################
#TODO: alle files einlesen und in ein grosses dict packen
# erst mal kann ich davon ausgehen dass alle files vom selben project kommen
#
# struktur:
#   {commitID: {'build-time': time, files: {filename: {record}, filename: {record}}}}
#   NICHT: {filename: {commitID: {record}, commitID: {record}}
#
################################################################################


def buildFullRecord(pathToRecords, pathToBuildTimes):
    '''Builds a complete record from all the single hash records.
       The records are grouped by the commitIDs'''
    fullRecord = {}

    with open(pathToBuildTimes, 'r') as buildTimesFile:
        buildTimes = eval(buildTimesFile.read())
        for commitID in buildTimes:
            fullRecord[commitID] = {}
            fullRecord[commitID]['build-time'] = buildTimes[commitID]
            fullRecord[commitID]['files'] = {}
            
        #TODO: sort entries by time and store them sorted
        # have to sort commits somehow
        # => sort by time (start with oldest)


    for recordFilename in getListOfFiles(pathToRecords):
        for line in open(recordFilename):
            data = eval(line)
            commitID = data['commit-hash']
            del data['commit-hash']
            filename = data['filename']
            del data['filename']       
            fullRecord[commitID]['files'][filename] = data

    return fullRecord

################################################################################

def makeBuildTimeGraph(fullRecord):
    commitNr = 0;
    iterCommits = iter(fullRecord)
    prevCommit = fullRecord[next(iterCommits)]

    f1 = open("/home/cip/2015/yb90ifym/clang-hash/build/muslHashes/times.csv", 'a')
   
    for commitID in iterCommits:
        currentCommit = fullRecord[commitID]
        totalOptimalRedundantCompileTime = 0 # ns
        totalASTHashRedundantCompileTime = 0 # ns
        currentFiles = currentCommit['files']
        prevFiles = prevCommit['files']
        for filename in currentFiles:
            if 'ast-hash' not in currentFiles[filename].keys():
                #TODO: counter?
                break
       
            currentRecord = currentFiles[filename]
            prevRecord = prevFiles[filename]
           
            if prevRecord['object-hash'] == currentRecord['object-hash']:
                totalOptimalRedundantCompileTime += currentRecord['compile-duration']
            if prevRecord['ast-hash'] == currentRecord['ast-hash']:
                totalASTHashRedundantCompileTime += currentRecord['compile-duration']

        buildTime = currentCommit['build-time'] # ns
        optimalBuildTime = buildTime - totalOptimalRedundantCompileTime # = buildTime - sum(compileTime(file) if objhash(file) unchanged)
        astHashBuildTime = buildTime - totalASTHashRedundantCompileTime # = buildTime - sum(compileTime(file) if asthash(file) unchanged)

        f1.write("%s;%s;%s;%s\n" % (commitNr, buildTime, optimalBuildTime, astHashBuildTime))

        commitNr += 1
        prevCommit = currentCommit        


    f1.close()

################################################################################
# main:
#TODO: paths!!!
if (len(sys.argv) > 1):
    pathToRecords = sys.argv[1]
    
    records = []
    for filename in getListOfFiles(pathToRecords):
        records = [eval(line) for line in open(filename)]
        validateHashes(records)
    print "Errors: %d, Infos: %d, Missing: %d" % (errorCount, astDifferObjSameCount, missingCount)



    fullRecord = buildFullRecord(pathToRecords, "/home/cip/2015/yb90ifym/clang-hash/build/muslHashes/buildTimes_musl.info")

    makeBuildTimeGraph(fullRecord)


#    f = open("/home/cip/2015/yb90ifym/clang-hash/build/muslHashes/fullRecord.info", 'a')
#    f.write(repr(fullRecord) + "\n")
#    f.close()


else:
    print "Missing path to record files.\nUsage:\n\t%s pathToRecords" % sys.argv[0]

