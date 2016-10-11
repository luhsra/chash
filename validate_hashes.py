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
import csv


SKIP_VALIDATING = True  #TODO: make command line arg

PATH_TO_RECORDS = '' # gets set from command line parameter

# data/record filenames
INFO_EXTENSION = '.info'
FULL_RECORD_FILENAME = 'fullRecord' + INFO_EXTENSION
COMMIT_INFO_FILENAME = 'commitInfo_musl' + INFO_EXTENSION

# graph filenames
PNG_EXTENSION = '.png'
PARSE_TIME_HISTOGRAM_FILENAME = 'parseTimeHistogram' + PNG_EXTENSION
HASH_TIME_HISTOGRAM_FILENAME = 'hashTimeHistogram' + PNG_EXTENSION
COMPILE_TIME_HISTOGRAM_FILENAME = 'compileTimeHistogram' + PNG_EXTENSION
BUILD_TIME_HISTOGRAM_FILENAME = 'buildTimeHistogram' + PNG_EXTENSION
CHANGES_GRAPH_FILENAME = 'changes' + PNG_EXTENSION
BUILD_TIMES_GRAPH_FILENAME = 'buildTimes' + PNG_EXTENSION
BUILD_TIME_COMPOSITION_FILENAME = 'buildTimeComposition' + PNG_EXTENSION

# CSV filenames
#TODO: put in dict for easier check if existing
CSV_EXTENSION = '.csv'
BUILD_TIME_COMPOSITION_DATA_FILENAME = 'buildTimeCompositionData' + CSV_EXTENSION
BUILD_TIME_DATA_FILENAME = 'buildTimeData' + CSV_EXTENSION
CHANGES_DATA_FILENAME = 'changesData' + CSV_EXTENSION
SINGLE_TIMES_DATA_FILENAME = 'singleTimesData' + CSV_EXTENSION

# CSV headers
BUILD_TIME_DATA_HEADER = ['measuredBuildTimes', 'realClangHashBuildTimes', 'optimalClangHashBuildTimes', 'optimalBuildTimes']
BUILD_TIME_COMPOSITION_DATA_HEADER = ['totalParseTimes', 'totalHashTimes', 'totalCompileTimes', 'diffToBuildTime']
CHANGES_DATA_HEADER = ['fileCount','sameHashes', 'differentAstHashes', 'differentObjHashes']
SINGLE_TIMES_DATA_HEADER = ['parsing', 'hashing', 'compiling']




def abs_path(filename):
    """Prepends the absolute path to the filename.
    """
    return PATH_TO_RECORDS + '/../' + filename


def getListOfFiles(directory):
    for root, dirnames, filenames in os.walk(directory):
        for filename in fnmatch.filter(filenames, '*' + INFO_EXTENSION):
            yield os.path.join(root, filename)


def printHashInfo(message, prevRecord, record, isError=True):
    print "%s: file %s, commits %s to %s : %s" % ("ERROR" if isError else "INFO", record['filename'], prevRecord['commit-hash'], record['commit-hash'], message)



errorCount = 0
astDifferObjSameCount = 0
missingCount = 0    


def validateRecords():
    for filename in getListOfFiles(PATH_TO_RECORDS):
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
#
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
    'deletions':        19,
    'other': 20 #TODO: remove, just 4 testing
}
keyTranslationFromNr = {v: k for k, v in keyTranslationToNr.items()}

keyTranslation = keyTranslationToNr.copy()
keyTranslation.update(keyTranslationFromNr)


def tr(key):
    """lookup key translation (both directions)"""
    return keyTranslation[key]


def buildFullRecordTo(pathToFullRecordFile):
    """structure of full record:
    {commitID: {'build-time': time, files: {filename: {record}, filename: {record}}}}
    """
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
    """Builds a complete record from all the single hash records.
    The records are grouped by the commitIDs
    """
    fullRecord = {}
    with open(abs_path(COMMIT_INFO_FILENAME), 'r') as commitInfoFile:
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

    for recordFilename in getListOfFiles(PATH_TO_RECORDS):
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


def plot_build_time_graph1(data):
    plotBuildTimeGraph(data[0], data[1], data[2], data[3])

def plotBuildTimeGraph(measuredBuildTimes, realClangHashBuildTimes, optimalClangHashBuildTimes, optimalBuildTimes): # times in s
    fig, ax = plt.subplots()

    ax.plot(measuredBuildTimes, label='measured build time')
    ax.plot(realClangHashBuildTimes, label='real clang-hash build time')
    ax.plot(optimalClangHashBuildTimes, label='optimal clang-hash build time')
    ax.plot(optimalBuildTimes, label='optimal build time')

    lgd = ax.legend(loc='center left', bbox_to_anchor=(1, 0.5)) # legend on the right

    plt.xlabel('commits')
    plt.ylabel('time [ms]')
    fig.savefig(abs_path(BUILD_TIMES_GRAPH_FILENAME), bbox_extra_artists=(lgd,), bbox_inches='tight')


def plot_build_time_composition_graph1(data):
    plotBuildTimeCompositionGraph(data[0], data[1], data[2], data[3])

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
    fig.savefig(abs_path(BUILD_TIME_COMPOSITION_FILENAME), bbox_extra_artists=(lgd,), bbox_inches='tight')


def plotTimeHistogram(times, filename):
    #TODO: understand params and vars
    hist, bins = np.histogram(times, bins=50)
    width = 0.7 * (bins[1] - bins[0])
    center = (bins[:-1] + bins[1:]) / 2
    fig, ax = plt.subplots()
    plt.xlabel('time [ms]')
    plt.ylabel('#files')
    ax.bar(center, hist, align='center', width=width)
    fig.savefig(filename)#, bbox_extra_artists=(lgd,), bbox_inches='tight')


def plotTimeMultiHistogram(parseTimes, hashTimes, compileTimes, filename):
    times = np.column_stack((parseTimes, hashTimes, compileTimes))
    fig, ax = plt.subplots()
    hist = ax.hist(times, bins=50, label=['parsing', 'hashing', 'compiling'])
    width = 0.7 * (bins[1] - bins[0])
    center = (bins[:-1] + bins[1:]) / 2

    plt.xlabel('time [ms]')
    plt.ylabel('#files')
    ax.bar(center, hist, align='center', width=width)
    fig.savefig(filename)#, bbox_extra_artists=(lgd,), bbox_inches='tight')


def plot_time_histograms1(data):
    plotTimeHistograms(data[0], data[1], data[2])

def plotTimeHistograms(parseTimes, hashTimes, compileTimes): # times in ms
    plotTimeHistogram(parseTimes, abs_path(PARSE_TIME_HISTOGRAM_FILENAME))
    plotTimeHistogram(hashTimes, abs_path(HASH_TIME_HISTOGRAM_FILENAME))
    plotTimeHistogram(compileTimes, abs_path(COMPILE_TIME_HISTOGRAM_FILENAME))
#    plotTimeMultiHistogram(parseTimes, hashTimes, compileTimes, abs_path(BUILD_TIME_HISTOGRAM_FILENAME))


def plot_changes_graph1(data):
    plotChangesGraph(data[0], data[1], data[2], data[3])

def plotChangesGraph(fileCounts, sameHashes, differentAstHashes, differentObjHashes):
    fig, ax = plt.subplots()

    ax.plot(fileCounts, label='#objfiles')
    ax.plot(sameHashes, label='unchanged')
    ax.plot(differentAstHashes, label='astHash differs')
    ax.plot(differentObjHashes, label='objHash differs')

    box = ax.get_position()
    lgd = ax.legend(loc='center left', bbox_to_anchor=(1, 0.5)) # legend on the right

    plt.xlabel('commits')
    plt.ylabel('#files')
    fig.savefig(abs_path(CHANGES_GRAPH_FILENAME), bbox_extra_artists=(lgd,), bbox_inches='tight')


################################################################################

def write_to_csv(data, columnNames, filename):
    with open(filename, "w") as csvFile:
        writer = csv.writer(csvFile)
        writer.writerow(columnNames)
        for line in data:
            writer.writerow(line)

################################################################################


def makeGraphs(fullRecord):
    sortedCommitIDs = getSortedCommitIDList(fullRecord)
    iterCommits = iter(sortedCommitIDs)
    prevCommitID = next(iterCommits)
    prevCommit = fullRecord[prevCommitID]

    # data for build time graphs
    measuredBuildTimes = []
    optimalBuildTimes = []
    optimalClangHashBuildTimes = []
    realClangHashBuildTimes = []

    totalParseTimes = []
    totalHashTimes = []
    totalCompileTimes = []
    diffToBuildTime = []

    #data for histograms
    parseTimes = []
    hashTimes = []
    compileTimes = []

    # data for changes graph
    differentAstHashes = []
    differentObjHashes = []
    sameHashes = []
    fileCounts = []

    missingFilesTotal = 0 # count how many files are added in commits, just 4 checking
    missingFileErrors = 0
    totalFilesChanged = 0

    currentFiles = prevCommit[tr('files')]
    totalFilesChanged += len(currentFiles)
    for filename in currentFiles: # deal with first commit
        if tr('ast-hash') not in currentFiles[filename].keys():
            print "error: missing AST hash for file %s" % filename
            continue
        currentRecord = currentFiles[filename]
        parseTimes.append(currentRecord[tr('parse-duration')] / 1e6) # ns to ms
        hashTimes.append(currentRecord[tr('hash-duration')] / 1e6) 
        compileTimes.append(currentRecord[tr('compile-duration')] / 1e6)

    for commitID in iterCommits:
        currentCommit = fullRecord[commitID]
        currentFiles = currentCommit[tr('files')]
        prevFiles = prevCommit[tr('files')]
  
        totalOptimalRedundantTime = 0 # ns
        totalOptimalRedundantCompileTime = 0 # ns
        totalASTHashRedundantCompileTime = 0 # ns
        totalCompileDuration = 0 # ns # incl. parsing time
        totalParseDuration = 0 # ns
        totalHashDuration = 0 # ns

        same = 0 #TODO: rename to ...Count?
        differentAstHash = 0
        differentObjHash = 0
        fileCount = 0

        missingFiles = 0

        totalFilesChanged += currentCommit[tr('files-changed')]

        for filename in currentFiles:
            if tr('ast-hash') not in currentFiles[filename].keys():
                print "error: missing AST hash for file %s" % filename
                continue
            prevFilename = filename
            if filename not in prevFiles:
                if 'src/' + filename in prevFiles:
                    print "file %s changed place to src/" % filename
                    prevFilename = 'src/' + filename
                    prevRecord = prevFiles[prevFilename]
                elif 'crt/' + filename in prevFiles:
                    print "file %s changed place to crt/" % filename
                    prevFilename = 'crt/' + filename
                    prevRecord = prevFiles[prevFilename]
                else:
                    print "MISSING: %s not in prev (%s), current is (%s)" % (filename, prevCommitID, commitID)
                    missingFilesTotal += 1
                    missingFiles += 1
                    continue


            currentRecord = currentFiles[filename]
            prevRecord = prevFiles[prevFilename]

            parseDuration = currentRecord[tr('parse-duration')] # ns
            hashDuration = currentRecord[tr('hash-duration')] # ns
            compileDuration = currentRecord[tr('compile-duration')] - parseDuration # ns

            totalParseDuration += parseDuration
            totalHashDuration += hashDuration
            totalCompileDuration += compileDuration
 

            if prevRecord[tr('ast-hash')] == currentRecord[tr('ast-hash')]:
                totalASTHashRedundantCompileTime += compileDuration # ns

            if prevRecord[tr('object-hash')] == currentRecord[tr('object-hash')]:
                totalOptimalRedundantTime += compileDuration + hashDuration + parseDuration #ns
                totalOptimalRedundantCompileTime += compileDuration
            else:
                # For the histograms, only take file into account if it changed, to prevent the same files to be counted multiple times
                parseTimes.append(currentRecord[tr('parse-duration')] / 1e6) # ns to ms
                hashTimes.append(currentRecord[tr('hash-duration')] / 1e6) 
                compileTimes.append(currentRecord[tr('compile-duration')] / 1e6) 


            # for changes graph
            if prevRecord[tr('object-hash')] != currentRecord[tr('object-hash')]:
                differentObjHash += 1
                differentAstHash += 1
            elif prevRecord[tr('ast-hash')] != currentRecord[tr('ast-hash')]:
                differentAstHash += 1
            else:
                same += 1

            fileCount += 1

        if missingFiles > currentCommit[tr('files-changed')]:
            print "!!!!FAIL!!!!"
            missingFileErrors += 1
            print "%s: missingFiles: %d, filesChanged: %d" % (commitID, missingFiles, currentCommit[tr('files-changed')])

        buildTime = currentCommit[tr('build-time')] # ns
        optimalBuildTime = buildTime - totalOptimalRedundantTime # = buildTime - sum((clangTime(file) + hashTime) if objhash(file) unchanged)
        realAstHashBuildTime = buildTime - totalASTHashRedundantCompileTime # = buildTime - sum(compileTime(file) if asthash(file) unchanged)
        optimalAstHashBuildTime = buildTime - totalOptimalRedundantCompileTime


        measuredBuildTimes.append(buildTime / 1e9) # nano to seconds
        optimalBuildTimes.append(optimalBuildTime / 1e9)
        optimalClangHashBuildTimes.append(optimalAstHashBuildTime / 1e9)
        realClangHashBuildTimes.append(realAstHashBuildTime / 1e9)

        totalParseTimes.append(totalParseDuration / 1e9) # nano to seconds
        totalHashTimes.append(totalHashDuration / 1e9)
        totalCompileTimes.append(totalCompileDuration / 1e9)
        diffToBuildTime.append((buildTime - totalParseDuration - totalHashDuration - totalCompileDuration) / 1e9)


        # changes graph
        differentAstHashes.append(differentAstHash)
        differentObjHashes.append(differentObjHash)
        sameHashes.append(same)
        fileCounts.append(fileCount)

        prevCommit = currentCommit
        prevCommitID = commitID


    print "missingFilesTotal: %d, missingFileErrors: %d" % (missingFilesTotal, missingFileErrors)
    print "totalFilesChanged: %d, sizes: parseTimes(%d), hashTimes(%d), compileTimes(%d)" % (totalFilesChanged, len(parseTimes), len(hashTimes), len(compileTimes))

    plotBuildTimeGraph(measuredBuildTimes, realClangHashBuildTimes, optimalClangHashBuildTimes, optimalBuildTimes)
    plotBuildTimeCompositionGraph(totalParseTimes, totalHashTimes, totalCompileTimes, diffToBuildTime)
    plotTimeHistograms(parseTimes, hashTimes, compileTimes)
    plotChangesGraph(fileCounts, sameHashes, differentAstHashes, differentObjHashes)

    # save data to csv files
    buildTimeData = np.column_stack((measuredBuildTimes, realClangHashBuildTimes, optimalClangHashBuildTimes, optimalBuildTimes))
    write_to_csv(buildTimeData, BUILD_TIME_DATA_HEADER, abs_path(BUILD_TIME_DATA_FILENAME))

    buildTimeCompositionData = np.column_stack((totalParseTimes, totalHashTimes, totalCompileTimes, diffToBuildTime))
    write_to_csv(buildTimeCompositionData, BUILD_TIME_COMPOSITION_DATA_HEADER, abs_path(BUILD_TIME_COMPOSITION_DATA_FILENAME))

    singleTimesData = np.column_stack((parseTimes, hashTimes, compileTimes))
    write_to_csv(singleTimesData, SINGLE_TIMES_DATA_HEADER, abs_path(SINGLE_TIMES_DATA_FILENAME))

    changesData = np.column_stack((fileCounts, sameHashes, differentAstHashes, differentObjHashes))
    write_to_csv(changesData, CHANGES_DATA_HEADER, abs_path(CHANGES_DATA_FILENAME))

################################################################################
"""functions for reading data from the csv files to skip full record building"""

def csv_files_are_existing():
    return (os.path.isfile(abs_path(BUILD_TIME_COMPOSITION_DATA_FILENAME))
            and os.path.isfile(abs_path(BUILD_TIME_DATA_FILENAME))
            and os.path.isfile(abs_path(CHANGES_DATA_FILENAME))
            and os.path.isfile(abs_path(SINGLE_TIMES_DATA_FILENAME)))

def read_from_csv(filename, columnNames):
    data = []
    with open(filename) as csv_file:
        reader = csv.reader(csv_file)
        is_header_row = True
        for row in reader:
            if is_header_row:
                for col in row:
                    data.append([])
                is_header_row = False
            else:
                colnum = 0
                for col in row:
                    data[colnum].append(float(col))
                    colnum += 1
    return data


def read_csv_data_and_plot_graphs():
    """Build the graphs from the data from the csv files from previous runs
    to save time by skipping the whole "full record" building.
    """

    build_time_data = read_from_csv(abs_path(BUILD_TIME_DATA_FILENAME), BUILD_TIME_DATA_HEADER)
    plot_build_time_graph1(build_time_data)


    build_time_composition_data = read_from_csv(abs_path(BUILD_TIME_COMPOSITION_DATA_FILENAME), BUILD_TIME_COMPOSITION_DATA_HEADER)
    plot_build_time_composition_graph1(build_time_composition_data)


    changes_data = read_from_csv(abs_path(CHANGES_DATA_FILENAME), CHANGES_DATA_HEADER)
    plot_changes_graph1(changes_data)


    single_times_data = read_from_csv(abs_path(SINGLE_TIMES_DATA_FILENAME), SINGLE_TIMES_DATA_HEADER)
    plot_time_histograms1(single_times_data)




################################################################################


# main:
if (len(sys.argv) > 1):
    PATH_TO_RECORDS = sys.argv[1]
    path_to_full_record_file = abs_path(FULL_RECORD_FILENAME)
    print "Starting at %s" % time.ctime()

    if not SKIP_VALIDATING:
        print "validating..."
        validateRecords()
        print "finished validating at %s" % time.ctime()

    if csv_files_are_existing():
        # skip building record, read csv data from files and work with that
        print "reading from csv files"
        read_csv_data_and_plot_graphs()
        print "finished graphs at %s" % time.ctime()
    else:
        full_record = buildFullRecordTo(path_to_full_record_file)
        print "finished building/loading full record at %s" % time.ctime()

        makeGraphs(full_record)
        print "finished graphs at %s" % time.ctime()

    print "Finished at %s" % time.ctime()
else:
    print "Missing path to record files.\nUsage:\n\t%s PATH_TO_RECORDS" % sys.argv[0]

