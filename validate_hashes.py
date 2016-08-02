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

def validateHashes(recordList):
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

        if prevRecord['ast-hash'] == record['ast-hash']:
            if prevRecord['object-hash'] != record['object-hash']:
                printHashInfo("ast hashes same, object hashes differ", prevRecord, record)
                errorCount += 1
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

pathToRecords = "" # gets set to command line parameter

################################################################################

def buildFullRecord():
    '''Builds a complete record from all the single hash records.
       The records are grouped by the commitIDs'''
    fullRecord = {}

    with open(pathToRecords + "/../buildTimes_musl.info", 'r') as buildTimesFile:
        buildTimes = eval(buildTimesFile.read())
        for commitID in buildTimes:
            fullRecord[commitID] = {}
            fullRecord[commitID]['build-time'] = buildTimes[commitID]
            fullRecord[commitID]['files'] = {}
            
        #TODO: sort entries by time and store them sorted
        # have to sort commits somehow
        # => sort by time (start with oldest)
        # atm already sorted, but not if parallelized

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

    with open(pathToRecords + "/../times.csv", 'w') as f_times:
        f_times.write("%s;%s;%s;%s;%s;%s;%s;%s\n" % ("commitNr", "buildTime", "optimalBuildTime", "astHashBuildTime", "compileTimeOnly", "withoutCompileTime", "totalParsingTime", "totalHashingTime"))
 
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
                    #TODO: counter?
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

            f_times.write("%s;%s;%s;%s;%s;%s;%s;%s\n" % (commitNr, buildTime, optimalBuildTime, astHashBuildTime, compileTimeOnly, buildTime - compileTimeOnly, totalParsingTime, buildTime - compileTimeOnly - totalParsingTime))

            commitNr += 1
            prevCommit = currentCommit        


################################################################################

def makeChangesGraph(fullRecord):
    commitNr = 0;
    iterCommits = iter(fullRecord)
    prevCommit = fullRecord[next(iterCommits)]

    with open(pathToRecords + "/../changes.csv", 'w') as f_changes:
        f_changes.write("%s;%s;%s;%s\n" % ("commitNr", "differentAstHash", "differentObjHash", "same"))
 
        for commitID in iterCommits:
            currentCommit = fullRecord[commitID]
            currentFiles = currentCommit['files']
            prevFiles = prevCommit['files']
            same = 0
            differentAstHash = 0
            differentObjHash = 0


            print currentFiles['testfile.c']
            for filename in currentFiles:
                if filename == 'testfile.c':
                    print "found testfile.c"

            for filename in currentFiles:
                if filename == 'testfile.c':
                    print "found testfile.c (2)"
                if 'ast-hash' not in currentFiles[filename].keys():
                    print "ast-hash not in keys of file " + filename
                    break
                currentRecord = currentFiles[filename]
                prevRecord = prevFiles[filename]

#                if prevRecord['object-hash'] == currentRecord['object-hash'] or prevRecord['ast-hash'] == currentRecord['ast-hash']:
#                    test = 0
#                else:
                if filename == 'testfile.c':
                    print prevRecord['object-hash']
                    print currentRecord['object-hash']
                    print prevRecord['ast-hash']
                    print currentRecord['ast-hash']
                    print '\n'

                if prevRecord['object-hash'] != currentRecord['object-hash']:
                    differentObjHash += 1
                    differentAstHash += 1
                elif prevRecord['ast-hash'] != currentRecord['ast-hash']:
                    differentAstHash += 1
                else:
                    same += 1
    
            f_changes.write("%s;%s;%s;%s\n" % (commitNr, differentAstHash, differentObjHash, same))
            #TODO: nicht als csv, sondern auch wieder als dict speichern!
            #ausserdem am besten auch den commit-hash mitspeichern
            commitNr += 1
            prevCommit = currentCommit


################################################################################


# main:
if (len(sys.argv) > 1):
    pathToRecords = sys.argv[1]
    
    for filename in getListOfFiles(pathToRecords):
        records = [eval(line) for line in open(filename)]
        validateHashes(records)
    print "Errors: %d, Infos: %d, Missing: %d" % (errorCount, astDifferObjSameCount, missingCount)



    fullRecord = buildFullRecord()

    makeBuildTimeGraph(fullRecord)
    makeChangesGraph(fullRecord)

    f = open(pathToRecords + "/../fullRecord.info", 'w')
    f.write(repr(fullRecord) + "\n")
    f.close()


else:
    print "Missing path to record files.\nUsage:\n\t%s pathToRecords" % sys.argv[0]

