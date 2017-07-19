#!/usr/bin/env python

import fnmatch
import os
import sys

OUTPUT_FLAG = '-o'
HELP_FLAG = '-h'

FUNCTION_PREFIX = 'function'
VARIABLE_PREFIX = 'variable'
INFO_EXTENSION = '.info'


def get_param_of(flag):
    index = sys.argv.index(flag) + 1
    if index < len(sys.argv):
        return sys.argv[index]


def get_list_of_info_files(directory):
    for root, dirnames, filenames in os.walk(directory):
        for filename in fnmatch.filter(filenames, '*' + INFO_EXTENSION):
            yield os.path.join(root, filename)


def get_record_from(info_filename):
    """ .info files can contain more than a single record, since new ones are appended by the wrapper script: only read most recent one
    """
    record = None
    try:
        with open(info_filename, 'r') as info_file:
            lines = info_file.read().splitlines()
            record = eval(lines[-1])
    except IOError: # have to catch this to prevent failing testcases when some
                    # .info files are deleted by another testcase while reading
        pass
    return record
 

def get_name_of(symbol):
    return symbol.split(':')[1]


def get_prefix_of(symbol):
    return symbol.split(':')[0]


def read_info_files(directory):
    local_hashes = {}
    used_definitions = {}

    for info_file in get_list_of_info_files(directory):
        record = get_record_from(info_file)
        if record:
            for elem in record['element-hashes']:
                prefix = get_prefix_of(elem[0])
                symbol = get_name_of(elem[0])

                if symbol in local_hashes:
                    if local_hashes[symbol] != elem[1]: # duplicate ok if hashes are the same
                        assert symbol not in local_hashes # otherwise, every symbol must be unique

                local_hashes[symbol] = elem[1]

                if len(elem) > 2 and (prefix == FUNCTION_PREFIX or prefix == VARIABLE_PREFIX):
                    used_definitions[symbol] = elem[2]
    return (local_hashes, used_definitions)
