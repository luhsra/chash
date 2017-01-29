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
FULL_RECORD_FILENAME = 'full_record' + INFO_EXTENSION
COMMIT_INFO_FILENAME = 'buildInfo_musl_with_stop' + INFO_EXTENSION
BUILD_TIME_DATA_FILENAME = 'total_build_times.csv'
BUILD_TIME_FILENAME = 'total_build_times.pdf'
BUILD_TIME_DATA_HEADER = ['total_parse_times', 'total_hash_times', 'total_compile_times', 'diff_to_build_times', 'total_build_times']


def abs_path(filename):
    """Prepends the absolute path to the filename."""
    return PATH_TO_RECORDS + '/../' + filename


def get_list_of_files(directory):
    for root, dirnames, filenames in os.walk(directory):
        for filename in fnmatch.filter(filenames, '*' + INFO_EXTENSION):
            yield os.path.join(root, filename)


################################################################################
#
#
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
    key_translation_from_nr = {v: k for k, v in key_translation_to_nr.items()}

    key_translation_dict = key_translation_to_nr.copy()
    key_translation_dict.update(key_translation_from_nr)

    return key_translation_dict


key_translation = build_key_translation_dict()

def tr(key):
    """lookup key translation (both directions)"""
    return key_translation[key]


def build_full_record_to(path_to_full_record_file):
    """structure of full record:
    {commitID: {'build-time': time, files: {filename: {record}, filename: {record}}}}
    """
    full_record = build_full_record()
    if DO_PRINT_RECORDS:
        f = open(path_to_full_record_file, 'w')
        try:
            f.write(repr(full_record) + "\n")
        except MemoryError as me:
            print me
            raise
        finally:
            print time.ctime()
            f.close()
        print "built full record, wrote to " + path_to_full_record_file

    return full_record


def build_full_record():
    """Builds a complete record from all the single hash records.
    The records are grouped by the commitIDs
    """
    full_record = {}
    with open(abs_path(COMMIT_INFO_FILENAME), 'r') as commit_infoFile:
        commit_info = eval(commit_infoFile.read())
        for run_id in commit_info:
            if not isinstance(run_id, int): # dict also contains key 'commit-hash'
                continue;
            current_record = {}
            current_record[tr('filename')] = commit_info[run_id]['filename']
            current_record[tr('build-time')] = commit_info[run_id]['build-time']
            current_record[tr('files')] = {}
            full_record[run_id] = current_record
            
    for record_filename in get_list_of_files(PATH_TO_RECORDS):
        for line in open(record_filename):
            data = eval(line)
#            commitID = data['commit-hash']
#            del data['commit-hash']
            
            obj_filename = data['obj-file']
            del data['obj-file']

            
            # del everything I don't need
            del data['return-code']
            del data['element-hashes']
            del data['project']
            del data['processed-bytes']
            del data['object-file-size']
            
            run_id = data['run_id']

            data_new_keys = {tr(k): v for k, v in data.items()}
            full_record[run_id][tr('files')][obj_filename] = data_new_keys

    return full_record


################################################################################

def write_to_csv(data, column_names, filename):
    with open(filename, "w") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(column_names)
        for line in data:
            writer.writerow(line)


def print_avg(data, name):
    print 'avg %s: %f' % (name, sum(data)/float(len(data)))

################################################################################

parse_color, hash_color, compile_color, remain_color = ('#FFFF66','#FF0000','#3399FF','#008800')

def plot_build_time_graph1(data):
    plot_build_time_composition_graph(data[0], data[1], data[2], data[3])


def plot_build_time_composition_graph(parse_times, hash_times, compile_times, diff_to_build_time): # times in ms
    fig, ax = plt.subplots()

    ax.stackplot(np.arange(1, len(parse_times)+1), # x axis
                 [parse_times, hash_times, compile_times,
                #diff_to_build_time
                ], colors=[parse_color,hash_color,compile_color,
                 #   remain_color
                ], edgecolor='none')
    plt.xlim(1,len(parse_times))
    plt.xlabel('commits')
    plt.ylabel('time [s]')
    ax.set_yscale('log')
    lgd = ax.legend([#mpatches.Patch(color=remain_color),
                     mpatches.Patch(color=compile_color),
                     mpatches.Patch(color=hash_color),
                     mpatches.Patch(color=parse_color)],
                    [#'remaining build time',
                    'compile time', 'hash time', 'parse time'],
                    loc='center left', bbox_to_anchor=(1, 0.5))
    fig.savefig(abs_path(BUILD_TIME_FILENAME), bbox_extra_artists=(lgd,), bbox_inches='tight')

    print "\n-----------------"
    print "average total times per build:"
    print_avg(parse_times, 'parse')
    print_avg(hash_times, 'hash')
    print_avg(compile_times, 'compile')
    print_avg(diff_to_build_time, 'remainder')
    print ""
    print "average times if header/source file touched"
    print "-----------------\n"



################################################################################

def make_graphs(full_record):
    # data for build time graphs
    total_parse_times = []
    total_hash_times = []
    total_compile_times = []
    total_build_times = []
    diff_to_build_times = []

    parse_times_header_touched = []
    parse_times_source_touched = []


#    freshBuildRecord = full_record[0]
    for run_id in full_record:
        if run_id < 2: # skip fresh build (and also 1st, seems to be buggy...)
            continue
    
        current_record = full_record[run_id]
        current_files = current_record[tr('files')]
        files_changed = len(current_files) # count changed files per run #TODO!

        print current_record[tr('filename')]

        total_parse_duration = 0
        total_hash_duration = 0
        total_compile_duration = 0
        for filename in current_files: # deal with first commit
#            if tr('ast-hash') not in current_files[filename].keys():
#                print "error: missing AST hash for file %s" % filename
#                continue
            current_file_record = current_files[filename]
            total_parse_duration += current_file_record[tr('parse-duration')]
            total_hash_duration += current_file_record[tr('hash-duration')]
            total_compile_duration += current_file_record[tr('compile-duration')]
     
#        if total_parse_duration == 0:# or (total_compile_duration/1e6) > 500000:
#            continue
  
        total_parse_times.append(total_parse_duration / 1e6) # nano to milli
        total_hash_times.append(total_hash_duration / 1e6)
        total_compile_times.append(total_compile_duration / 1e6)
        build_time = current_record[tr('build-time')]
        total_build_times.append(build_time / 1e6)
        diff_to_build_times.append((build_time - total_parse_duration - total_hash_duration - total_compile_duration) / 1e6)

 
        print 'run_id %d, #files_changed: %d' % (run_id, files_changed)

    print_avg(total_build_times, 'total')

    # save data to csv files
    build_time_data = np.column_stack((total_parse_times, total_hash_times, total_compile_times, diff_to_build_times, total_build_times))
    write_to_csv(build_time_data, BUILD_TIME_DATA_HEADER, abs_path(BUILD_TIME_DATA_FILENAME))

    plot_build_time_composition_graph(total_parse_times, total_hash_times, total_compile_times, diff_to_build_times)
    
################################################################################
"""functions for reading data from the csv files to skip full record building"""

def csv_files_are_existing():
    return os.path.isfile(abs_path(BUILD_TIME_DATA_FILENAME))

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
    full_record = build_full_record_to(path_to_full_record_file)
    print "finished building/loading full record at %s" % time.ctime()

    make_graphs(full_record)
    print "finished graphs at %s" % time.ctime()

    print "Finished at %s" % time.ctime()
else:
    print "Missing path to record files.\nUsage:\n\t%s PATH_TO_RECORDS" % sys.argv[0]

