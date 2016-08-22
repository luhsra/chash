#!/usr/bin/env python

import fnmatch
import os
import sys
from operator import itemgetter

FULLRECORDFILENAME = "/../fullRecord.info"
COMMITINFOFILENAME = "/../commitInfo_musl.info"
#TODO: define all filenames here
pathToRecords = "" # gets set to command line parameter



def getListOfFiles(directory):
    for root, dirnames, filenames in os.walk(directory):
        for filename in fnmatch.filter(filenames, '*.info'):
            yield os.path.join(root, filename)


def printHashInfo(message, prevRecord, record, isError=True):
    print "%s: file %s, commits %s to %s : %s" % ("ERROR" if isError else "INFO", record['filename'], prevRecord['commit-hash'], record['commit-hash'], message)



errorCount = 0
astDifferObjSameCount = 0
missingCount = 0    


def validateRecords():
    for filename in getListOfFiles(pathToRecords):
        records = [eval(line) for line in open(filename)]
        validateHashes(records)
    print "Errors: %d, Infos: %d, Missing: %d" % (errorCount, astDifferObjSameCount, missingCount)

 
def validateHashes(recordList):
    #TODO: also sort this, perhaps execute on fullRecords or check against sorted commitIDs
    global errorCount, astDifferObjSameCount, missingCount
    #TODO: this method assumes that all records are from the same object file
    recordList.reverse() # glaube die sind im moment falsch herum sortiert (neuester als erstes)
    iterRecords = iter(recordList)
    prevRecord = next(iterRecords)
    filename = prevRecord['filename']

    if 'ast-hash' not in prevRecord.keys():
        #print "MISSING: no ast-hash in records for file " + filename
        missingCount += 1
        return
    
    for record in iterRecords:
#TODO        if prevRecord['start-time'] > record['start-time']:
#TODO            print "Error: wrong order of records"
            #TODO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        if 'ast-hash' not in record.keys() or 'object-hash' not in record.keys():
            print "ERROR: stopping validating for file %s; no ast-hash available for commit %s" % (filename, record['commit-hash'])
            break

#        if prevRecord['ast-hash'] == record['ast-hash']:
#            if prevRecord['object-hash'] != record['object-hash']:
#                printHashInfo("ast hashes same, object hashes differ", prevRecord, record)
#                errorCount += 1
        #TODO: deckt nicht das obere if auch alle faelle vom unteren mit ab?
        #TODO: refactoren, das obere kann man weglassen...
        if prevRecord['object-hash'] != record['object-hash']:
            if prevRecord['ast-hash'] == record['ast-hash']:
                printHashInfo("object hashes differ, ast hashes same", prevRecord, record)
                errorCount += 1
        elif prevRecord['ast-hash'] != record['ast-hash']:
            #printHashInfo("ast hashes differ, object hashes same", prevRecord, record, False)
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


def buildFullRecord():
    '''Builds a complete record from all the single hash records.
       The records are grouped by the commitIDs'''
    fullRecord = {}

    with open(pathToRecords + COMMITINFOFILENAME, 'r') as commitInfoFile:
        commitInfo = eval(commitInfoFile.read())
        #TODO: replace fillRecord[commitID] und commitInfo[commitID] with local vars/refs
        for commitID in commitInfo:
            fullRecord[commitID] = {}
            fullRecord[commitID]['commit-time'] = commitInfo[commitID]['commit-time']
            fullRecord[commitID]['build-time'] = commitInfo[commitID]['build-time']
            fullRecord[commitID]['files'] = {}
            fullRecord[commitID]['filesChanged'] = commitInfo[commitID]['filesChanged']
            
            if 'insertions' in commitInfo[commitID]:
                fullRecord[commitID]['insertions'] = commitInfo[commitID]['insertions']
            if 'deletions' in commitInfo[commitID]:
                fullRecord[commitID]['deletions'] = commitInfo[commitID]['deletions']
        
    for recordFilename in getListOfFiles(pathToRecords):
        for line in open(recordFilename):
            data = eval(line)
            commitID = data['commit-hash']
            del data['commit-hash']
            filename = data['filename']
            del data['filename']       
            fullRecord[commitID]['files'][filename] = data

    return fullRecord


def getSortedCommitIDList(fullRecord):
    return sorted(fullRecord, key=lambda x: (fullRecord[x]['commit-time']))


################################################################################

def makeBuildTimeGraph(fullRecord):
    commitNr = 0;
    sortedCommitIDs = getSortedCommitIDList(fullRecord)
    iterCommits = iter(sortedCommitIDs)
    prevCommit = fullRecord[next(iterCommits)]

    with open(pathToRecords + "/../times.csv", 'w') as f_times:
        f_times.write("%s;%s;%s;%s;%s;%s;%s;%s\n" % ("commitHash", "buildTime", "optimalBuildTime", "astHashBuildTime", "compileTimeOnly", "withoutCompileTime", "totalParsingTime", "totalHashingTime"))
 
        for commitID in iterCommits:
            currentCommit = fullRecord[commitID]
            totalOptimalRedundantCompileTime = 0 # ns
            totalASTHashRedundantCompileTime = 0 # ns
            currentFiles = currentCommit['files']
            prevFiles = prevCommit['files']
            compileTimeOnly = 0 # ns
            totalParsingTime = 0 # ns
        
            for filename in currentFiles:
                if 'ast-hash' not in currentFiles[filename].keys():
                    break
                currentRecord = currentFiles[filename]
                prevRecord = prevFiles[filename]
            
                compileTimeOnly += currentRecord['compile-duration'] # ns
                totalParsingTime += currentRecord['parse-duration'] # ns

                if prevRecord['object-hash'] == currentRecord['object-hash']:
                    totalOptimalRedundantCompileTime += currentRecord['compile-duration'] # ns
                if prevRecord['ast-hash'] == currentRecord['ast-hash']:
                    totalASTHashRedundantCompileTime += currentRecord['compile-duration'] # ns

            buildTime = currentCommit['build-time'] # ns
            optimalBuildTime = buildTime - totalOptimalRedundantCompileTime # = buildTime - sum(compileTime(file) if objhash(file) unchanged)
            astHashBuildTime = buildTime - totalASTHashRedundantCompileTime # = buildTime - sum(compileTime(file) if asthash(file) unchanged)

            f_times.write("%s;%s;%s;%s;%s;%s;%s;%s\n" % (commitID, buildTime, optimalBuildTime, astHashBuildTime, compileTimeOnly, buildTime - compileTimeOnly, totalParsingTime, buildTime - compileTimeOnly - totalParsingTime))

            commitNr += 1
            prevCommit = currentCommit        


################################################################################

def makeChangesGraph(fullRecord):
    commitNr = 0;
    sortedCommitIDs = getSortedCommitIDList(fullRecord)
    iterCommits = iter(sortedCommitIDs)
    prevCommit = fullRecord[next(iterCommits)]

    with open(pathToRecords + "/../changes.csv", 'w') as f_changes:
        f_changes.write("%s;%s;%s;%s\n" % ("commitHash", "differentAstHash", "differentObjHash", "same"))
 
        for commitID in iterCommits:
            currentCommit = fullRecord[commitID]
            currentFiles = currentCommit['files']
            prevFiles = prevCommit['files']
            same = 0
            differentAstHash = 0
            differentObjHash = 0
            print commitID

            for filename in currentFiles:
                if 'ast-hash' not in currentFiles[filename].keys():
                    if filename[-2:] != '.s':
                        print "ast-hash not in keys of file " + filename
                    continue
                currentRecord = currentFiles[filename]
                if filename not in prevFiles:
                    if 'src/' + filename in prevFiles:
                        print "file %s changed place to src/" % filename
                        prevRecord = prevFiles['src/' + filename]
                    else:
                        print "ERROR, MISSING FILE: %s not in prev, going on to next commit" % filename
                        continue
                else:
                    prevRecord = prevFiles[filename]

                if prevRecord['object-hash'] != currentRecord['object-hash']:
                    differentObjHash += 1
                    differentAstHash += 1
                elif prevRecord['ast-hash'] != currentRecord['ast-hash']:
                    differentAstHash += 1
                else:
                    same += 1
    
            f_changes.write("%s;%s;%s;%s\n" % (commitID, differentAstHash, differentObjHash, same))
            #TODO: don't save as csv, but also as dict!
            commitNr += 1
            prevCommit = currentCommit


################################################################################


# main:
if (len(sys.argv) > 1):
    pathToRecords = sys.argv[1]
    pathToFullRecordFile = pathToRecords + FULLRECORDFILENAME

    validateRecords()

    fullRecord = {}
    if os.path.isfile(pathToFullRecordFile):
        with open(pathToFullRecordFile, 'r') as fullRecordFile:
            fullRecord = eval(fullRecordFile.read())
            print "read full record from " + pathToFullRecordFile
    else:
        fullRecord = buildFullRecord()
        f = open(pathToFullRecordFile, 'w')
        f.write(repr(fullRecord) + "\n")
        f.close()
        print "built full record, wrote to" + pathToFullRecordFile

    makeBuildTimeGraph(fullRecord)
    print "finished BuildTimeGraph"
    makeChangesGraph(fullRecord)
    print "finished ChangesGraph"

else:
    print "Missing path to record files.\nUsage:\n\t%s pathToRecords" % sys.argv[0]

