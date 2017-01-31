#!/usr/bin/env python

import matplotlib
matplotlib.use('Agg')

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

PATH_TO_RECORDS = '' # gets set from command line parameter

# data/record filenames
INFO_EXTENSION = '.info'
FULL_RECORD_FILENAME = 'fullRecord' + INFO_EXTENSION
COMMIT_INFO_FILENAME = 'commitInfo_musl' + INFO_EXTENSION

# graph filenames
GRAPH_EXTENSION = '.pdf' #TODO: rename to sth. like GRAPH_FILE_EXTENSION
PARSE_TIME_HISTOGRAM_FILENAME = 'parseTimeHistogram' + GRAPH_EXTENSION
HASH_TIME_HISTOGRAM_FILENAME = 'hashTimeHistogram' + GRAPH_EXTENSION
COMPILE_TIME_HISTOGRAM_FILENAME = 'compileTimeHistogram' + GRAPH_EXTENSION
BUILD_TIME_HISTOGRAM_FILENAME = 'buildTimeHistogram' + GRAPH_EXTENSION
CHANGES_GRAPH_FILENAME = 'changes' + GRAPH_EXTENSION
BUILD_TIMES_GRAPH_FILENAME = 'buildTimes' + GRAPH_EXTENSION
BUILD_TIME_COMPOSITION_FILENAME = 'buildTimeComposition' + GRAPH_EXTENSION

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
    """Prepends the absolute path to the filename."""
    return PATH_TO_RECORDS + '/../' + filename


def get_list_of_files(directory):
    for root, dirnames, filenames in os.walk(directory):
        for filename in fnmatch.filter(filenames, '*' + INFO_EXTENSION):
            yield os.path.join(root, filename)

def write_to_csv(data, column_names, filename):
    with open(filename, "w") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(column_names)
        for line in data:
            writer.writerow(line)

def write_to_file(data, filename):
    with open(abs_path(filename), 'w') as f:
        try:
            f.write(repr(data))
        except MemoryError as me:
            print me
            raise


def plot_hash_count_histogram(hash_values, filename):
    dictionary = plt.figure()
    fig, ax = plt.subplots()
    plt.xlabel('nr of different hashes')
    plt.ylabel('nr of files')
    ax.bar(hash_values.keys(), hash_values.values(), align='center')
    fig.savefig(filename)


false_negatives = 0
ast_hash_missing = 0

source_files = set() # all filenames of the hashed files
ast_hashes_dict = {} # maps filename -> set(ast hashes)
obj_hashes_dict = {} # maps filename -> set(obj hashes)

unchanged_counter = 0

ast_hashes_set = set()
obj_hashes_set = set()

nr_of_records = 0 # for calculating avg
sum_of_times = {'parsing': 0,
             'hashing': 0,
             'compiling': 0}

false_positive_records = {} # maps commitID -> set of source filenames of false positive records
all_ast_hashes = {} # maps commitID -> set(ast hashes)
all_obj_hashes = {} # maps commitID -> set(obj hashes)
all_files_hash_contained = {} # maps commitID -> set(filename), to prevent multiple hashes from the same file in one commit to be counted twice
defect_commits = 0
commit_ids = set()



def validate_records():
    total_number_of_records = 0
    for filename in get_list_of_files(PATH_TO_RECORDS):
        with open(filename) as f:
            records = list(reversed([eval(line) for line in f]))
            total_number_of_records += len(records)
            validate_hashes(records)

    different_ast_hashes = 0 # number of different ast hashes (in total)
    for v in ast_hashes_dict.values():
        different_ast_hashes += len(v)

    different_obj_hashes = 0 # number of different obj hashes (in total)
    for v in obj_hashes_dict.values():
        different_obj_hashes += len(v)

    #TODO: remove old calculation of false positives
    false_positives = 0
    for k in false_positive_records:
        false_positives += len(false_positive_records[k])
    
    print "\n---- Results ----"
    print "nr of commits: %d" % len(commit_ids)
    print "nr of single records (compiler calls): %d" % total_number_of_records
    print "false negatives (errors): %d" % false_negatives
    print "false positives: %d" % false_positives
    print "missing ast hashes: %d" % ast_hash_missing
    print ""
    print "source files: %d" % len(source_files)
    print "different AST hashes: %d" % different_ast_hashes
    print "different obj hashes: %d" % different_obj_hashes
    print "size of ast_hashes_dict: %d, obj_hashes_dict: %d" % (len(ast_hashes_dict), len(obj_hashes_dict))
    print "len(ast_hashes_set): %d, len(obj_hashes_set): %d" % (len(ast_hashes_set), len(obj_hashes_set))
    print "unchanged_counter: %d" % unchanged_counter
    print ""
    print "avg times:"
    for k,v in sum_of_times.items():
        print "%s: %d ns" % (k, v/nr_of_records)
    print "-----------------"
    print "false positive commits: %d" % len(false_positive_records)
    for k in sorted(false_positive_records, key=lambda k: len(false_positive_records[k]), reverse=True):
        print "\t%s: %d" % (k, len(false_positive_records[k]))
    print "-----------------\n"


    diff_ast_hashes = 0
    for v in all_ast_hashes.values():
        diff_ast_hashes += len(v)
    
    diff_obj_hashes = 0
    for v in all_obj_hashes.values():
        diff_obj_hashes += len(v)

    print ">>> corrected >>>"
    print "different AST hashes: %d" % diff_ast_hashes
    print "different obj hashes: %d" % diff_obj_hashes
    print "<<<<<<<<<<<<<<<<<"
    
    write_to_csv([ [k,len(v)] for k,v in ast_hashes_dict.items() ], ['filename', 'nr of different hashes'], abs_path('different_ast_hashes_per_file.csv'))
    write_to_csv([ [k,len(v)] for k,v in obj_hashes_dict.items() ], ['filename', 'nr of different hashes'], abs_path('different_obj_hashes_per_file.csv'))


def print_hash_info(message, prev_record, record, is_error=True):
    print "%s: file %s, commits %s to %s : %s" % ("ERROR" if is_error else "INFO", record['filename'], prev_record['commit-hash'], record['commit-hash'], message)


def validate_hashes(record_list):
    ''' All records in the list must be from the same object file'''
    #TODO: collect data from all files before validating (changing paths src and crt) and sort

    global false_negatives, ast_hash_missing
    global source_files
    global ast_hashes_dict, obj_hashes_dict
    global ast_hashes_set, obj_hashes_set
    global sum_of_times, nr_of_records
    global unchanged_counter

    global all_ast_hashes, all_obj_hashes, all_files_hash_contained

    iter_records = iter(record_list)
    prev_record = next(iter_records)
    filename = prev_record['filename']
    source_files.add(filename)

    if 'ast-hash' not in prev_record.keys():
        #print "MISSING: no ast-hash in records for file " + filename
        ast_hash_missing += 1
        return



    # the different hashes of the current file
    ast_hashes = set()
    obj_hashes = set()

    ast_hashes.add(prev_record['ast-hash'])
    obj_hashes.add(prev_record['object-hash'])
    ast_hashes_set.add(prev_record['ast-hash'])
    obj_hashes_set.add(prev_record['object-hash'])

    # setup for counting different ast/obj hashes:
    commit_hash = prev_record['commit-hash']
    if commit_hash not in all_ast_hashes:
        all_ast_hashes[commit_hash] = set() 
    if commit_hash not in all_obj_hashes:
        all_obj_hashes[commit_hash] = set() 
    if commit_hash not in all_files_hash_contained:
        all_files_hash_contained[commit_hash] = set()

    if filename not in all_files_hash_contained[commit_hash]:
        all_ast_hashes[commit_hash].add(prev_record['ast-hash'])
        all_obj_hashes[commit_hash].add(prev_record['object-hash']) 
        all_files_hash_contained[commit_hash].add(filename)


    commit_ids.add(prev_record['commit-hash'])

    nr_of_records += 1
    sum_of_times['parsing'] += prev_record['parse-duration']
    sum_of_times['hashing'] += prev_record['hash-duration']
    sum_of_times['compiling'] += prev_record['compile-duration'] - (prev_record['parse-duration'] + prev_record['hash-duration'])


    for record in iter_records:
        
        #if prev_record['start-time'] > record['start-time']:
        #    print "Error: wrong order of records" #TODO: fix, then remove this
        if 'ast-hash' not in record.keys() or 'object-hash' not in record.keys():
            print "ERROR: stopping validating for file %s; no ast-hash available for commit %s" % (filename, record['commit-hash'])
            break

        if prev_record['object-hash'] != record['object-hash']:
            if prev_record['ast-hash'] == record['ast-hash']:
                print_hash_info("object hashes differ, ast hashes same", prev_record, record)
                false_negatives += 1
        elif prev_record['ast-hash'] != record['ast-hash']:
            #print_hash_info("ast hashes differ, object hashes same", prev_record, record, False) #TODO: include this and look at the changes
            commit_hash = record['commit-hash']
            if commit_hash not in false_positive_records:
                false_positive_records[commit_hash] = set()
            false_positive_records[commit_hash].add(record['filename'])
        else:
            unchanged_counter += 1

        ast_hashes.add(record['ast-hash'])
        obj_hashes.add(record['object-hash'])

        ast_hashes_set.add(record['ast-hash'])
        obj_hashes_set.add(record['object-hash'])

        # now the correct approach:
        if commit_hash not in all_ast_hashes:
            all_ast_hashes[commit_hash] = set()
        if commit_hash not in all_obj_hashes:
            all_obj_hashes[commit_hash] = set()
        if commit_hash not in all_files_hash_contained:
            all_files_hash_contained[commit_hash] = set()
        
        if filename not in all_files_hash_contained[commit_hash]:
            if prev_record['ast-hash'] != record['ast-hash']:
                all_ast_hashes[commit_hash].add(record['ast-hash'])
                all_files_hash_contained[commit_hash].add(filename)
            if prev_record['object-hash'] != record['object-hash']:
                all_obj_hashes[commit_hash].add(record['object-hash'])    
                all_files_hash_contained[commit_hash].add(filename)



        nr_of_records += 1
        sum_of_times['parsing'] += record['parse-duration']
        sum_of_times['hashing'] += record['hash-duration']
        sum_of_times['compiling'] += record['compile-duration'] - (record['parse-duration'] + record['hash-duration'])

        commit_ids.add(record['commit-hash'])
        
        prev_record = record


    if filename in ast_hashes_dict:
        ast_hashes_dict[filename] |=ast_hashes # merge sets
    else:
        ast_hashes_dict[filename] = ast_hashes

    if filename in obj_hashes_dict:
        obj_hashes_dict[filename] |= obj_hashes # merge sets
    else:
        obj_hashes_dict[filename] = obj_hashes


    different_ast_hash_count = len(ast_hashes)
    different_obj_hash_count = len(obj_hashes)


################################################################################

def build_key_translation_dict():
    key_translation_to_nr = {
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
        'compile-duration': 10, # time the compiler was running (incl. parse-duration) #TODO: AND hash-duration?
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
    key_translation_from_nr = {v: k for k, v in key_translation_to_nr.items()}

    key_translation_dict = key_translation_to_nr.copy()
    key_translation_dict.update(key_translation_from_nr)

    return key_translation_dict


key_translation = build_key_translation_dict()

def tr(key):
    """lookup key translation (both directions)"""
    return key_translation[key]


def build_full_record_to(pathToFullRecordFile):
    """structure of full record:
    {commitID: {'build-time': time, files: {filename: {record}, filename: {record}}}}
    """
    full_record = {}
    # this leads to being Killed by OS due to tremendous memory consumtion...
    #if os.path.isfile(pathToFullRecordFile):
    #    with open(pathToFullRecordFile, 'r') as fullRecordFile:
    #        print "loading full record from " + pathToFullRecordFile
    #        full_record = eval(fullRecordFile.read())
    #        print "read full record from " + pathToFullRecordFile
    #else:
    full_record = build_full_record()
#    f = open(pathToFullRecordFile, 'w')
#    try:
#        f.write(repr(full_record) + "\n")
#    except MemoryError as me:
#        print me
#        raise
#    finally:
#        print time.ctime()
#        f.close()
#    print "built full record, wrote to " + pathToFullRecordFile
    return full_record


def build_full_record():
    """Builds a complete record from all the single hash records.
    The records are grouped by the commitIDs
    """
    full_record = {}
    with open(abs_path(COMMIT_INFO_FILENAME), 'r') as commitInfoFile:
        commitInfo = eval(commitInfoFile.read())
        for commitID in commitInfo:
            full_record[commitID] = {}
            full_record[commitID][tr('commit-time')] = commitInfo[commitID]['commit-time']
            #print commitID
            if 'build-time' in commitInfo[commitID]:
                full_record[commitID][tr('build-time')] = commitInfo[commitID]['build-time']
            else:
                full_record[commitID][tr('build-time')] = 0
            full_record[commitID][tr('files')] = {}
            full_record[commitID][tr('files-changed')] = commitInfo[commitID]['files-changed']
            
            if 'insertions' in commitInfo[commitID]:
                full_record[commitID][tr('insertions')] = commitInfo[commitID]['insertions']
            if 'deletions' in commitInfo[commitID]:
                full_record[commitID][tr('deletions')] = commitInfo[commitID]['deletions']

    for recordFilename in get_list_of_files(PATH_TO_RECORDS):
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
            full_record[commitID][tr('files')][objFilename] = dataNewKeys

    return full_record

################################################################################

def get_sorted_commit_id_list(full_record):
    return sorted(full_record, key=lambda x: (full_record[x][tr('commit-time')]))

################################################################################


def plot_build_time_graph1(data):
    plot_build_time_graph(data[0], data[1], data[2], data[3])

def plot_build_time_graph(measuredBuildTimes, realClangHashBuildTimes, optimalClangHashBuildTimes, optimalBuildTimes): # times in s
    fig, ax = plt.subplots()

    ax.plot([i/60 for i in measuredBuildTimes], label='measured build time')
    ax.plot([i/60 for i in realClangHashBuildTimes], label='real clang-hash build time')
    ax.plot([i/60 for i in optimalClangHashBuildTimes], label='optimal clang-hash build time')
    ax.plot([i/60 for i in optimalBuildTimes], label='optimal build time')

    lgd = ax.legend(loc='center left', bbox_to_anchor=(1, 0.5)) # legend on the right
    ax.set_ylim([0,5])
    plt.xlabel('commits')
    plt.ylabel('time [min]')
    fig.savefig(abs_path(BUILD_TIMES_GRAPH_FILENAME), bbox_extra_artists=(lgd,), bbox_inches='tight')


def plot_build_time_composition_graph1(data):
    plot_build_time_composition_graph(data[0], data[1], data[2], data[3])

def print_avg(data, name):
    print 'avg %s: %f' % (name, sum(data)/float(len(data)))

parseColor, hashColor, compileColor, remainColor = ('#FFFF66','#FF0000','#3399FF','#008800') 

def plot_build_time_composition_graph(parseTimes, hashTimes, compileTimes, diffToBuildTime): # times in s
    fig, ax = plt.subplots()

    ax.stackplot(np.arange(1, len(parseTimes)+1), # x axis
#                 [parseTimes, hashTimes, compileTimes, diffToBuildTime],
                  [[i/60 for i in parseTimes], [i/60 for i in hashTimes], [i/60 for i in compileTimes], [i/60 for i in diffToBuildTime]],
                 colors=[parseColor,hashColor,compileColor,remainColor], edgecolor='none')
    plt.xlim(1,len(parseTimes))
    plt.xlabel('commits')
    plt.ylabel('time [min]')
    lgd = ax.legend([mpatches.Patch(color=remainColor),
                     mpatches.Patch(color=compileColor),
                     mpatches.Patch(color=hashColor),
                     mpatches.Patch(color=parseColor)],
                    ['remaining build time','compile time', 'hash time', 'parse time'],
                    loc='center left', bbox_to_anchor=(1, 0.5))
    fig.savefig(abs_path(BUILD_TIME_COMPOSITION_FILENAME), bbox_extra_artists=(lgd,), bbox_inches='tight')
    print_avg(parseTimes, 'parse')
    print_avg(hashTimes, 'hash')
    print_avg(compileTimes, 'compile')
    print_avg(diffToBuildTime, 'remainder')


def plotTimeHistogram(times, filename): # times in ms
    #TODO: understand params and vars
    hist, bins = np.histogram([i/1000 for i in times], bins=50) # times to s
    width = 0.7 * (bins[1] - bins[0])
    center = (bins[:-1] + bins[1:]) / 2
    fig, ax = plt.subplots()
    plt.xlabel('time [s]')
    plt.ylabel('#files')
    ax.bar(center, hist, align='center', width=width)
    fig.savefig(filename)


def plotTimeMultiHistogram(parseTimes, hashTimes, compileTimes, filename): # times in ms
    bins = np.linspace(0, 5000, 50)
    data = np.vstack([parseTimes, hashTimes, compileTimes]).T
    fig, ax = plt.subplots()
    plt.hist(data, bins, alpha=0.7, label=['parsing', 'hashing', 'compiling'], color=[parseColor, hashColor, compileColor])
    plt.legend(loc='upper right')
    plt.xlabel('time [ms]')
    plt.ylabel('#files')
    fig.savefig(filename)

    fig, ax = plt.subplots()
    boxplot_data = [[i/1000 for i in parseTimes], [i/1000 for i in hashTimes], [i/1000 for i in compileTimes]] # times to s
    plt.boxplot(boxplot_data, 0, 'rs', 0, [5, 95])
    plt.xlabel('time [s]')
    plt.yticks([1, 2, 3], ['parsing', 'hashing', 'compiling'])
    #lgd = ax.legend(loc='center left', bbox_to_anchor=(1, 0.5)) # legend on the right
    fig.savefig(filename[:-4] + '_boxplots' + GRAPH_EXTENSION)



def plot_time_histograms1(data):
    plotTimeHistograms(data[0], data[1], data[2])

def plotTimeHistograms(parseTimes, hashTimes, compileTimes): # times in ms
    plotTimeHistogram(parseTimes, abs_path(PARSE_TIME_HISTOGRAM_FILENAME))
    plotTimeHistogram(hashTimes, abs_path(HASH_TIME_HISTOGRAM_FILENAME))
    plotTimeHistogram(compileTimes, abs_path(COMPILE_TIME_HISTOGRAM_FILENAME))
    plotTimeMultiHistogram(parseTimes, hashTimes, compileTimes, abs_path(BUILD_TIME_HISTOGRAM_FILENAME))


def plot_changes_graph1(data):
    plotChangesGraph(data[0], data[1], data[2], data[3])

def plotChangesGraph(fileCounts, sameHashes, differentAstHashes, differentObjHashes):
    fig, ax = plt.subplots()
    
    #('#FFFF66','#FF0000','#3399FF','#008800') 
    ax.plot(fileCounts, label='#objfiles', color='#EEAD0E')#'black')
    #ax.plot(sameHashes, label='unchanged')#, color='blue')
    ax.plot(differentAstHashes, label='astHash differs', color=compileColor)#'#000088')
    ax.plot(differentObjHashes, label='objHash differs', color=hashColor)#'#FFFF00') #'#0099FF')

    box = ax.get_position()
    lgd = ax.legend(loc='center left', bbox_to_anchor=(1, 0.5)) # legend on the right

    plt.xlabel('commits')
    plt.ylabel('#files')
    fig.savefig(abs_path(CHANGES_GRAPH_FILENAME), bbox_extra_artists=(lgd,), bbox_inches='tight')


################################################################################

broken_commits = set()

def make_graphs(full_record):

    global broken_commits

    sortedCommitIDs = get_sorted_commit_id_list(full_record)
    iterCommits = iter(sortedCommitIDs)
    prevCommitID = next(iterCommits)
    prevCommit = full_record[prevCommitID]

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
        currentCommit = full_record[commitID]
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
            fileCount += 1
            if tr('ast-hash') not in currentFiles[filename].keys():
                print "error: missing AST hash for file %s" % filename
                continue
            prevFilename = filename
            if filename not in prevFiles:
                if 'src/' + filename in prevFiles:
                    print "file %s changed place to src/" % filename  #TODO: is this actually necessary?
                    prevFilename = 'src/' + filename
                elif 'crt/' + filename in prevFiles:
                    print "file %s changed place to crt/" % filename
                    prevFilename = 'crt/' + filename
                else:
                    #print "MISSING: %s not in prev (%s), current is (%s)" % (filename, prevCommitID, commitID)
                    missingFilesTotal += 1
                    missingFiles += 1
                    continue


            currentRecord = currentFiles[filename]
            prev_record = prevFiles[prevFilename]

            parseDuration = currentRecord[tr('parse-duration')] # ns
            hashDuration = currentRecord[tr('hash-duration')] # ns
            compileDuration = currentRecord[tr('compile-duration')] - parseDuration # ns

            totalParseDuration += parseDuration
            totalHashDuration += hashDuration
            totalCompileDuration += compileDuration
 

            if prev_record[tr('ast-hash')] == currentRecord[tr('ast-hash')]:
                totalASTHashRedundantCompileTime += compileDuration # ns

            if prev_record[tr('object-hash')] == currentRecord[tr('object-hash')]:
                totalOptimalRedundantTime += compileDuration + hashDuration + parseDuration #ns
                totalOptimalRedundantCompileTime += compileDuration
            else:
                # For the histograms, only take file into account if it changed, to prevent the same files to be counted multiple times
                parseTimes.append(currentRecord[tr('parse-duration')] / 1e6) # ns to ms
                hashTimes.append(currentRecord[tr('hash-duration')] / 1e6) 
                compileTimes.append(currentRecord[tr('compile-duration')] / 1e6) 


            # for changes graph
            if prev_record[tr('object-hash')] != currentRecord[tr('object-hash')]:
                differentObjHash += 1
                differentAstHash += 1
            elif prev_record[tr('ast-hash')] != currentRecord[tr('ast-hash')]:
                differentAstHash += 1
            else:
                same += 1


        if missingFiles > currentCommit[tr('files-changed')]:
            #print "!!!!FAIL!!!!"
            missingFileErrors += 1
            #print "%s: missingFiles: %d, filesChanged: %d" % (commitID, missingFiles, currentCommit[tr('files-changed')])

        buildTime = currentCommit[tr('build-time')] # ns
        optimalBuildTime = buildTime - totalOptimalRedundantTime # = buildTime - sum((clangTime(file) + hashTime) if objhash(file) unchanged)
        realAstHashBuildTime = buildTime - totalASTHashRedundantCompileTime # = buildTime - sum(compileTime(file) if asthash(file) unchanged)
        optimalAstHashBuildTime = buildTime - totalOptimalRedundantCompileTime


        #TODO: remove broken commits; ok?
        if buildTime > 3e12 and totalParseDuration/1e9 > 300:
            measuredBuildTimes.append(buildTime / 16e9) # nano to seconds; also /16 to account for make -j16
            optimalBuildTimes.append(optimalBuildTime / 16e9)
            optimalClangHashBuildTimes.append(optimalAstHashBuildTime / 16e9)
            realClangHashBuildTimes.append(realAstHashBuildTime / 16e9)

            totalParseTimes.append(totalParseDuration / 16e9) # nano to seconds
            totalHashTimes.append(totalHashDuration / 16e9)
            totalCompileTimes.append(totalCompileDuration / 16e9)
            diffToBuildTime.append((buildTime - totalParseDuration - totalHashDuration - totalCompileDuration) / 16e9)


            # changes graph
            differentAstHashes.append(differentAstHash)
            differentObjHashes.append(differentObjHash)
            sameHashes.append(same)
            fileCounts.append(fileCount)
        else:
            broken_commits.add(commitID)

        prevCommit = currentCommit
        prevCommitID = commitID
        if fileCount == 0:
            print "no filecount at %s" % commitID

    print "missingFilesTotal: %d, missingFileErrors: %d" % (missingFilesTotal, missingFileErrors)
    print "totalFilesChanged: %d, sizes: parseTimes(%d), hashTimes(%d), compileTimes(%d)" % (totalFilesChanged, len(parseTimes), len(hashTimes), len(compileTimes))

    plot_build_time_graph(measuredBuildTimes, realClangHashBuildTimes, optimalClangHashBuildTimes, optimalBuildTimes)
    plot_build_time_composition_graph(totalParseTimes, totalHashTimes, totalCompileTimes, diffToBuildTime)
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

def read_from_csv(filename, column_names):
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


    if '--skip-validating' not in sys.argv:
        print "validating..."
        validate_records()
        print "finished validating at %s" % time.ctime()


    if csv_files_are_existing():
        # skip building record, read csv data from files and work with that
        print "reading from csv files"
        read_csv_data_and_plot_graphs()
        print "finished graphs at %s" % time.ctime()
    else:
        full_record = build_full_record_to(path_to_full_record_file)
        print "finished building/loading full record at %s" % time.ctime()

        make_graphs(full_record)
        print "finished graphs at %s" % time.ctime()

    print "broken commits: %d" % len(broken_commits)
    for commit in broken_commits:
        print "\t" + commit

    print "Finished at %s" % time.ctime()
else:
    print "Missing path to record files.\nUsage:\n\t%s PATH_TO_RECORDS" % sys.argv[0]

