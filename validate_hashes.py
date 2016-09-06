#!/usr/bin/env python

import fnmatch
import os
import sys
from operator import itemgetter
import time
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np
import matplotlib.mlab as mlab


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
    #also TODO: collect data from all files before validating (changing paths src and crt)
    #TODO: validate return-code!
    global errorCount, astDifferObjSameCount, missingCount
    #TODO: this method assumes that all records are from the same object file
    iterRecords = iter(recordList)
    prevRecord = next(iterRecords)
    filename = prevRecord['filename']

    if 'ast-hash' not in prevRecord.keys():
        #print "MISSING: no ast-hash in records for file " + filename
        missingCount += 1
        return
    
    for record in iterRecords:
        if prevRecord['start-time'] > record['start-time']:
            print "Error: wrong order of records" #TODO: fix, then remove this
        if 'ast-hash' not in record.keys() or 'object-hash' not in record.keys():
            print "ERROR: stopping validating for file %s; no ast-hash available for commit %s" % (filename, record['commit-hash'])
            break

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


keyTranslationToNr = {
    'start-time':       0,
    'hash-start-time':  1,
    'object-hash':      2,
    'return-code':      3,
    'parse-duration':   4,
    'object-file-size': 5,
    'processed-bytes':  6,
    'hash-duration':    7,
    'filename':         8,
    'project':          9,
    'compile-duration': 10, # time the compiler was running (incl. parse-duration)
    'ast-hash':         11,
    'commit-hash':      12,
    'element-hashes':   13,
    'commit-time':      14,
    'build-time':       15, # time the 'make -jx' command took, times x 
    'files':            16,
    'files-changed':    17,
    'insertions':       18,
    'deletions':        19
}
keyTranslationFromNr = {v: k for k, v in keyTranslationToNr.items()}

keyTranslation = keyTranslationToNr.copy()
keyTranslation.update(keyTranslationFromNr)


def tr(key):
    return keyTranslation[key]


def buildFullRecordTo(pathToFullRecordFile):
    fullRecord = {}
    # this leads to being Killed by OS due to tremendous memory consumtion...
    #if os.path.isfile(pathToFullRecordFile):
    #    with open(pathToFullRecordFile, 'r') as fullRecordFile:
    #        print "loading full record from " + pathToFullRecordFile
    #        fullRecord = eval(fullRecordFile.read())
    #        print "read full record from " + pathToFullRecordFile
    #else:
    fullRecord = buildFullRecord()
#    f = open(pathToFullRecordFile, 'w')
#    try:
#        f.write(repr(fullRecord) + "\n")
#    except MemoryError as me:
#        print me
#        raise
#    finally:
#        print time.ctime()
#        f.close()
#    print "built full record, wrote to " + pathToFullRecordFile
    return fullRecord


def buildFullRecord():
    '''Builds a complete record from all the single hash records.
       The records are grouped by the commitIDs'''
    fullRecord = {}
    with open(pathToRecords + COMMITINFOFILENAME, 'r') as commitInfoFile:
        commitInfo = eval(commitInfoFile.read())
        for commitID in commitInfo:
            fullRecord[commitID] = {}
            fullRecord[commitID][tr('commit-time')] = commitInfo[commitID]['commit-time']
            fullRecord[commitID][tr('build-time')] = commitInfo[commitID]['build-time']
            fullRecord[commitID][tr('files')] = {}
            fullRecord[commitID][tr('files-changed')] = commitInfo[commitID]['files-changed']
            
            if 'insertions' in commitInfo[commitID]:
                fullRecord[commitID][tr('insertions')] = commitInfo[commitID]['insertions']
            if 'deletions' in commitInfo[commitID]:
                fullRecord[commitID][tr('deletions')] = commitInfo[commitID]['deletions']

    for recordFilename in getListOfFiles(pathToRecords):
        for line in open(recordFilename):
            data = eval(line)
            commitID = data['commit-hash']
            del data['commit-hash']
            
            objFilename = data['obj-file']
            del data['obj-file']

            
            # del everything I don't need
            del data['return-code']
            del data['element-hashes']
            del data['project']
            del data['processed-bytes']
            del data['object-file-size']
            
            dataNewKeys = {tr(k): v for k, v in data.items()} 
            fullRecord[commitID][tr('files')][objFilename] = dataNewKeys

    return fullRecord

################################################################################

def getSortedCommitIDList(fullRecord):
    return sorted(fullRecord, key=lambda x: (fullRecord[x][tr('commit-time')]))

################################################################################

def plotBuildTimeGraph(measuredBuildTimes, realClangHashBuildTimes, optimalClangHashBuildTimes, optimalBuildTimes): # times in ms
    fig, ax = plt.subplots()

    ax.plot(measuredBuildTimes, label='measured build time')
    ax.plot(realClangHashBuildTimes, label='real clang-hash build time')
    ax.plot(optimalClangHashBuildTimes, label='optimal clang-hash build time')
    ax.plot(optimalBuildTimes, label='optimal build time')

    lgd = ax.legend(loc='center left', bbox_to_anchor=(1, 0.5)) # legend on the right

    plt.xlabel('commits')
    plt.ylabel('time [ms]')
    fig.savefig(pathToRecords + '/../buildTimes.png', bbox_extra_artists=(lgd,), bbox_inches='tight')


def plotBuildTimeCompositionGraph(parseTimes, hashTimes, compileTimes, diffToBuildTime): # times in s
    fig, ax = plt.subplots()
    
    ax.stackplot(np.arange(1, len(parseTimes)+1), # x axis
                 [parseTimes, hashTimes, compileTimes, diffToBuildTime],
                 colors=['#008800','#FF0000','#0000FF', '#000000'])
    plt.xlim(1,len(parseTimes))
    plt.xlabel('commits')
    plt.ylabel('time [s]')
    lgd = ax.legend([mpatches.Patch(color='#000000'),
                     mpatches.Patch(color='#0000FF'),
                     mpatches.Patch(color='#FF0000'),
                     mpatches.Patch(color='#008800')],
                    ['remaining build time','compile time', 'hash time', 'parse time'],
                    loc='center left', bbox_to_anchor=(1, 0.5))
    fig.savefig(pathToRecords + '/../buildTimeComposition.png', bbox_extra_artists=(lgd,), bbox_inches='tight')



def makeBuildTimeGraph(fullRecord):
    sortedCommitIDs = getSortedCommitIDList(fullRecord)
    iterCommits = iter(sortedCommitIDs)
    prevCommit = fullRecord[next(iterCommits)]

    measuredBuildTimes = []
    optimalBuildTimes = []
    optimalClangHashBuildTimes = []
    realClangHashBuildTimes = []

    parseTimes = []
    hashTimes = []
    compileTimes = []
    diffToBuildTime = []

    #f_times = open(pathToRecords + "/../times.csv", 'w')
    #f_times.write("%s;%s;%s;%s;%s;%s;%s;%s\n" % ("commitHash", "buildTime", "optimalBuildTime", "astHashBuildTime", "compileTimeOnly", "withoutCompileTime", "totalParsingTime", "totalHashingTime"))
 
    for commitID in iterCommits:
        currentCommit = fullRecord[commitID]
        totalOptimalRedundantTime = 0 # ns
        totalOptimalRedundantCompileTime = 0 # ns
        totalASTHashRedundantCompileTime = 0 # ns
        currentFiles = currentCommit[tr('files')]
        prevFiles = prevCommit[tr('files')]
        totalCompileDuration = 0 # ns # incl. parsing time #TODO:!!!
        totalParseDuration = 0 # ns
        totalHashDuration = 0 # ns
        for filename in currentFiles:
            if tr('ast-hash') not in currentFiles[filename].keys():
                #TODO: s.u., fix!
                continue
            if filename not in prevFiles:
                continue

            currentRecord = currentFiles[filename]
            prevRecord = prevFiles[filename]

            parseDuration = currentRecord[tr('parse-duration')] # ns
            hashDuration = currentRecord[tr('hash-duration')] # ns
            compileDuration = currentRecord[tr('compile-duration')] - parseDuration # ns

            totalParseDuration += parseDuration
            totalHashDuration += hashDuration
            totalCompileDuration += compileDuration
 
            if prevRecord[tr('object-hash')] == currentRecord[tr('object-hash')]:
                totalOptimalRedundantTime += compileDuration + hashDuration + parseDuration #ns
                totalOptimalRedundantCompileTime += compileDuration
            if prevRecord[tr('ast-hash')] == currentRecord[tr('ast-hash')]:
                totalASTHashRedundantCompileTime += compileDuration # ns

        buildTime = currentCommit[tr('build-time')] # ns
        optimalBuildTime = buildTime - totalOptimalRedundantTime # = buildTime - sum((clangTime(file) + hashTime) if objhash(file) unchanged)
        realAstHashBuildTime = buildTime - totalASTHashRedundantCompileTime # = buildTime - sum(compileTime(file) if asthash(file) unchanged)
        optimalAstHashBuildTime = buildTime - totalOptimalRedundantCompileTime

        #f_times.write("%s;%s;%s;%s;%s;%s;%s;%s\n" % (commitID, buildTime, optimalBuildTime, astHashBuildTime, totalCompileDuration, buildTime - compileTimeOnly, totalParseDuration, totalHashDuration))


        measuredBuildTimes.append(buildTime / 1e6) # nano to milli
        optimalBuildTimes.append(optimalBuildTime / 1e6)
        optimalClangHashBuildTimes.append(optimalAstHashBuildTime / 1e6)
        realClangHashBuildTimes.append(realAstHashBuildTime / 1e6)

        parseTimes.append(totalParseDuration / 1e9) # nano to seconds
        hashTimes.append(totalHashDuration / 1e9)
        compileTimes.append(totalCompileDuration / 1e9)
        diffToBuildTime.append((buildTime - totalParseDuration - totalHashDuration - totalCompileDuration) / 1e9)

        prevCommit = currentCommit

    plotBuildTimeGraph(measuredBuildTimes, realClangHashBuildTimes, optimalClangHashBuildTimes, optimalBuildTimes)

    plotBuildTimeCompositionGraph(parseTimes, hashTimes, compileTimes, diffToBuildTime)


################################################################################

def makeTimeHistograms(fullRecord):
    #TODO: for parsing, hashing, compiling
    sortedCommitIDs = getSortedCommitIDList(fullRecord)
    iterCommits = iter(sortedCommitIDs)
    prevCommit = fullRecord[next(iterCommits)]

    #TODO: den ersten commit auch behandeln!!!

    parseTimes = []
    hashTimes = []
    compileTimes = []

    currentFiles = prevCommit[tr('files')]
    for filename in currentFiles:
        if tr('ast-hash') not in currentFiles[filename].keys():
            print "error: missing AST hash for file %s" % filename
            continue
        parseTime = currentFiles[filename][tr('parse-duration')] / 1e6 # ns to ms
        parseTimes.append(parseTime) 
        hashTime = currentFiles[filename][tr('hash-duration')] / 1e6
        hashTimes.append(hashTime) 
        compileTime = currentFiles[filename][tr('compile-duration')] / 1e6
        compileTimes.append(compileTime) 



    for commitID in iterCommits:
        currentCommit = fullRecord[commitID] 
        currentFiles = currentCommit[tr('files')]
        prevFiles = prevCommit[tr('files')]

        for filename in currentFiles:
            if tr('ast-hash') not in currentFiles[filename].keys():
                print "error: missing AST hash for file %s" % filename
                continue
            if filename not in prevFiles:
                continue
    
            currentRecord = currentFiles[filename]
            prevRecord = prevFiles[filename]

            if prevRecord[tr('object-hash')] != currentRecord[tr('object-hash')]:
                parseTime = currentFiles[filename][tr('parse-duration')] / 1e6 # ns to ms
                parseTimes.append(parseTime) 
                hashTime = currentFiles[filename][tr('hash-duration')] / 1e6
                hashTimes.append(hashTime) 
                compileTime = currentFiles[filename][tr('compile-duration')] / 1e6
                compileTimes.append(compileTime) 




    #TODO: understand params and vars
    hist, bins = np.histogram(parseTimes, bins=50)
    width = 0.7 * (bins[1] - bins[0])
    center = (bins[:-1] + bins[1:]) / 2
    fig, ax = plt.subplots()
    plt.xlabel('time [ms]')
    plt.ylabel('#files')
    ax.bar(center, hist, align='center', width=width)
    fig.savefig(pathToRecords + '/../parseTimeHistogram.png')#, bbox_extra_artists=(lgd,), bbox_inche    s='tight')


    hist, bins = np.histogram(hashTimes, bins=50)
    width = 0.7 * (bins[1] - bins[0])
    center = (bins[:-1] + bins[1:]) / 2
    fig, ax = plt.subplots()
    plt.xlabel('time [ms]')
    plt.ylabel('#files')
    ax.bar(center, hist, align='center', width=width)
    fig.savefig(pathToRecords + '/../hashTimeHistogram.png')#, bbox_extra_artists=(lgd,), bbox_inche    s='tight')


    hist, bins = np.histogram(compileTimes, bins=50)
    width = 0.7 * (bins[1] - bins[0])
    center = (bins[:-1] + bins[1:]) / 2
    fig, ax = plt.subplots()
    plt.xlabel('time [ms]')
    plt.ylabel('#files')
    ax.bar(center, hist, align='center', width=width)
    fig.savefig(pathToRecords + '/../compileTimeHistogram.png')#, bbox_extra_artists=(lgd,), bbox_inche    s='tight')




################################################################################

def makeChangesGraph(fullRecord):
    sortedCommitIDs = getSortedCommitIDList(fullRecord)
    iterCommits = iter(sortedCommitIDs)
    prevCommit = fullRecord[next(iterCommits)]

    differentAstHashes = []
    differentObjHashes = []
    sameHashes = []
    fileCounts = []

#    f_changes = open(pathToRecords + "/../changes.csv", 'w')
#    f_changes.write("%s;%s;%s;%s\n" % ("commitHash", "differentAstHash", "differentObjHash", "same"))
 
    for commitID in iterCommits:
        currentCommit = fullRecord[commitID]
        currentFiles = currentCommit[tr('files')]
        prevFiles = prevCommit[tr('files')]
        same = 0
        differentAstHash = 0
        differentObjHash = 0
        fileCount = 0

        for filename in currentFiles:
            if tr('ast-hash') not in currentFiles[filename].keys():
                continue
            currentRecord = currentFiles[filename]
            if filename not in prevFiles:
                if 'src/' + filename in prevFiles:
                    print "file %s changed place to src/" % filename
                    prevRecord = prevFiles['src/' + filename]
                elif 'crt/' + filename in prevFiles:
                    print "file %s changed place to crt/" % filename
                    prevRecord = prevFiles['crt/' + filename]
                else:
                    print "ERROR, MISSING FILE: %s not in prev, continue with next commit" % filename
                    continue
            else:
                prevRecord = prevFiles[filename]

            if prevRecord[tr('object-hash')] != currentRecord[tr('object-hash')]:
                differentObjHash += 1
                differentAstHash += 1
            elif prevRecord[tr('ast-hash')] != currentRecord[tr('ast-hash')]:
                differentAstHash += 1
            else:
                same += 1

            fileCount += 1

        differentAstHashes.append(differentAstHash)
        differentObjHashes.append(differentObjHash)
        sameHashes.append(same)
        fileCounts.append(fileCount)

#        f_changes.write("%s;%s;%s;%s\n" % (commitID, differentAstHash, differentObjHash, same))
        prevCommit = currentCommit

    fig, ax = plt.subplots()

    ax.plot(fileCounts, label='#objfiles')
    ax.plot(sameHashes, label='unchanged')
    ax.plot(differentAstHashes, label='astHash differs')
    ax.plot(differentObjHashes, label='objHash differs')

    box = ax.get_position()
    lgd = ax.legend(loc='center left', bbox_to_anchor=(1, 0.5)) # legend on the right

    plt.xlabel('commits')
    plt.ylabel('#files')
    fig.savefig(pathToRecords + '/../changes.png', bbox_extra_artists=(lgd,), bbox_inches='tight')

################################################################################


# main:
if (len(sys.argv) > 1):
    pathToRecords = sys.argv[1]
    pathToFullRecordFile = pathToRecords + FULLRECORDFILENAME
    print "Starting at %s" % time.ctime()

#    validateRecords()
#    print "finished validating at %s" % time.ctime()

    fullRecord = buildFullRecordTo(pathToFullRecordFile)
    print "finished building/loading full record at %s" % time.ctime()

    makeBuildTimeGraph(fullRecord)
    print "finished BuildTimeGraph at %s" % time.ctime()

    makeTimeHistograms(fullRecord)
    print "finished timeHistograms at %s" % time.ctime()

    makeChangesGraph(fullRecord)
    print "finished ChangesGraph at %s" % time.ctime()
    
    print "Finished at %s" % time.ctime()
else:
    print "Missing path to record files.\nUsage:\n\t%s pathToRecords" % sys.argv[0]

