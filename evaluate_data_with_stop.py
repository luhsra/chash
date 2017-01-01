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

DO_PRINT_RECORDS = False
PATH_TO_RECORDS = '' # gets set from command line parameter

# data/record filenames
INFO_EXTENSION = '.info'
FULL_RECORD_FILENAME = 'fullRecord' + INFO_EXTENSION
COMMIT_INFO_FILENAME = 'buildInfo_musl_with_stop' + INFO_EXTENSION
BUILD_TIME_DATA_FILENAME = 'totalBuildTimes.csv'
BUILD_TIME_FILENAME = 'totalBuildTimes.pdf'
BUILD_TIME_DATA_HEADER = ['totalParseTimes', 'totalHashTimes', 'totalCompileTimes', 'diffToBuildTimes', 'totalBuildTimes']


def abs_path(filename):
    """Prepends the absolute path to the filename.
    """
    return PATH_TO_RECORDS + '/../' + filename


def getListOfFiles(directory):
    for root, dirnames, filenames in os.walk(directory):
        for filename in fnmatch.filter(filenames, '*' + INFO_EXTENSION):
            yield os.path.join(root, filename)


errorCount = 0
astDifferObjSameCount = 0
missingCount = 0    

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
    'run_id':           20
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
    fullRecord = buildFullRecord()   
    if DO_PRINT_RECORDS:
        f = open(pathToFullRecordFile, 'w')
        try:
            f.write(repr(fullRecord) + "\n")
        except MemoryError as me:
            print me
            raise
        finally:
            print time.ctime()
            f.close()
        print "built full record, wrote to " + pathToFullRecordFile

    return fullRecord


def buildFullRecord():
    """Builds a complete record from all the single hash records.
    The records are grouped by the commitIDs
    """
    fullRecord = {}
    with open(abs_path(COMMIT_INFO_FILENAME), 'r') as commitInfoFile:
        commitInfo = eval(commitInfoFile.read())
        for run_id in commitInfo:
            if not isinstance(run_id, int): # dict also contains key 'commit-hash'
                continue;
            currentRecord = {}
            currentRecord[tr('filename')] = commitInfo[run_id]['filename']
            currentRecord[tr('build-time')] = commitInfo[run_id]['build-time']
            currentRecord[tr('files')] = {}
            fullRecord[run_id] = currentRecord 
            
    for recordFilename in getListOfFiles(PATH_TO_RECORDS):
        for line in open(recordFilename):
            data = eval(line)
#            commitID = data['commit-hash']
#            del data['commit-hash']
            
            objFilename = data['obj-file']
            del data['obj-file']

            
            # del everything I don't need
            del data['return-code']
            del data['element-hashes']
            del data['project']
            del data['processed-bytes']
            del data['object-file-size']
            
            run_id = data['run_id']

            dataNewKeys = {tr(k): v for k, v in data.items()} 
            fullRecord[run_id][tr('files')][objFilename] = dataNewKeys

    return fullRecord


################################################################################

def write_to_csv(data, columnNames, filename):
    with open(filename, "w") as csvFile:
        writer = csv.writer(csvFile)
        writer.writerow(columnNames)
        for line in data:
            writer.writerow(line)


def printAvg(data, name):
    print 'avg %s: %f' % (name, sum(data)/float(len(data)))

################################################################################

parseColor, hashColor, compileColor, remainColor = ('#FFFF66','#FF0000','#3399FF','#008800')

def plot_build_time_graph1(data):
    plotBuildTimeCompositionGraph(data[0], data[1], data[2], data[3])


def plotBuildTimeCompositionGraph(parseTimes, hashTimes, compileTimes, diffToBuildTime): # times in ms
    fig, ax = plt.subplots()

    ax.stackplot(np.arange(1, len(parseTimes)+1), # x axis
                 [parseTimes, hashTimes, compileTimes,
                #diffToBuildTime
                ], colors=[parseColor,hashColor,compileColor,
                 #   remainColor
                ], edgecolor='none')
    plt.xlim(1,len(parseTimes))
    plt.xlabel('commits')
    plt.ylabel('time [s]')
    ax.set_yscale('log')
    lgd = ax.legend([#mpatches.Patch(color=remainColor),
                     mpatches.Patch(color=compileColor),
                     mpatches.Patch(color=hashColor),
                     mpatches.Patch(color=parseColor)],
                    [#'remaining build time',
                    'compile time', 'hash time', 'parse time'],
                    loc='center left', bbox_to_anchor=(1, 0.5))
    fig.savefig(abs_path(BUILD_TIME_FILENAME), bbox_extra_artists=(lgd,), bbox_inches='tight')

    printAvg(parseTimes, 'parse')
    printAvg(hashTimes, 'hash')
    printAvg(compileTimes, 'compile')
    printAvg(diffToBuildTime, 'remainder')



################################################################################

def makeGraphs(fullRecord):
    # data for build time graphs
    totalParseTimes = []
    totalHashTimes = []
    totalCompileTimes = []
    totalBuildTimes = []
    diffToBuildTimes = []

#    freshBuildRecord = fullRecord[0]
    for run_id in fullRecord:
        if run_id < 2: # skip fresh build (and also 1st, seems to be buggy...)
            continue
    
        currentRecord = fullRecord[run_id]
        currentFiles = currentRecord[tr('files')]
        filesChanged = len(currentFiles) # count changed files per run #TODO!

        print currentRecord[tr('filename')]

        totalParseDuration = 0
        totalHashDuration = 0
        totalCompileDuration = 0
        for filename in currentFiles: # deal with first commit
#            if tr('ast-hash') not in currentFiles[filename].keys():
#                print "error: missing AST hash for file %s" % filename
#                continue
            currentFileRecord = currentFiles[filename]
            totalParseDuration += currentFileRecord[tr('parse-duration')]
            totalHashDuration += currentFileRecord[tr('hash-duration')] 
            totalCompileDuration += currentFileRecord[tr('compile-duration')]
     
#        if totalParseDuration == 0:# or (totalCompileDuration/1e6) > 500000:
#            continue
  
        totalParseTimes.append(totalParseDuration / 1e6) # nano to milli
        totalHashTimes.append(totalHashDuration / 1e6)
        totalCompileTimes.append(totalCompileDuration / 1e6)
        buildTime = currentRecord[tr('build-time')]
        totalBuildTimes.append(buildTime / 1e6)
        diffToBuildTimes.append((buildTime - totalParseDuration - totalHashDuration - totalCompileDuration) / 1e6)

 
        print 'run_id %d, #filesChanged: %d' % (run_id, filesChanged)

    printAvg(totalBuildTimes, 'total')

    # save data to csv files
    buildTimeData = np.column_stack((totalParseTimes, totalHashTimes, totalCompileTimes, diffToBuildTimes, totalBuildTimes))
    write_to_csv(buildTimeData, BUILD_TIME_DATA_HEADER, abs_path(BUILD_TIME_DATA_FILENAME))

    plotBuildTimeCompositionGraph(totalParseTimes, totalHashTimes, totalCompileTimes, diffToBuildTimes)
    
################################################################################
"""functions for reading data from the csv files to skip full record building"""

def csv_files_are_existing():
    return os.path.isfile(abs_path(BUILD_TIME_DATA_FILENAME))

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


################################################################################


# main:
if (len(sys.argv) > 1):
    PATH_TO_RECORDS = sys.argv[1]
    path_to_full_record_file = abs_path(FULL_RECORD_FILENAME)
    print "Starting at %s" % time.ctime()

#    if csv_files_are_existing():
#        # skip building record, read csv data from files and work with that
#        print "reading from csv files"
#        read_csv_data_and_plot_graphs()
#        print "finished graphs at %s" % time.ctime()
#    else:
    full_record = buildFullRecordTo(path_to_full_record_file)
    print "finished building/loading full record at %s" % time.ctime()

    makeGraphs(full_record)
    print "finished graphs at %s" % time.ctime()

    print "Finished at %s" % time.ctime()
else:
    print "Missing path to record files.\nUsage:\n\t%s PATH_TO_RECORDS" % sys.argv[0]

