#!/usr/bin/env python

import fnmatch
import os
import sys

OUTPUT_FLAG = '-o'
HELP_FLAG = '-h'

FUNCTION_PREFIX = 'function'
STATIC_FUNCTION_PREFIX = 'static function'
VARIABLE_PREFIX = 'variable'
INFO_EXTENSION = '.info'


def static_vars(**kwargs):
    '''To enable C-style static function members'''
    def decorate(func):
        for k in kwargs:
            setattr(func, k, kwargs[k])
        return func
    return decorate


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
    """Name consists of symbol [1] and filename [2]"""
    elements =  symbol.split(':')[1:3]
    return ':'.join(elements)


def get_prefix_of(symbol):
    return symbol.split(':')[0]

def has_function_prefix(symbol):
    return get_prefix_of(symbol) in {FUNCTION_PREFIX, STATIC_FUNCTION_PREFIX}


def read_info_files(directory):
    # map function -> local hash
    local_hashes = {}
    # map function -> list of used defs
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


def read_info_files_functions_only(directory):
    # map function -> local hash
    local_hashes = {}
    # map function -> list of used defs. In this case, this is the call graph
    used_definitions = {}

    for info_file in get_list_of_info_files(directory):
        record = get_record_from(info_file)
        if record:
            for elem in record['element-hashes']:
                if not has_function_prefix(elem[0]):
                    continue

                symbol = get_name_of(elem[0])
                if symbol in local_hashes:
                    if local_hashes[symbol] != elem[1]: # duplicate ok if hashes are the same
                        assert symbol not in local_hashes # otherwise, every symbol must be unique

                local_hashes[symbol] = elem[1]

                if len(elem) > 2:
                    used_definitions[symbol] = [get_name_of(sym) for sym in elem[2]]
    return (local_hashes, used_definitions)

'''
def read_info_files_with_prefix(directory):
    # map function -> local hash
    local_hashes = {}
    # map function -> list of used defs
    used_definitions = {}

    for info_file in get_list_of_info_files(directory):
        record = get_record_from(info_file)
        if record:
            for elem in record['element-hashes']:
                prefix = get_prefix_of(elem[0])
                symbol = elem[0]

                if symbol in local_hashes:
                    if local_hashes[symbol] != elem[1]: # duplicate ok if hashes are the same
                        assert symbol not in local_hashes # otherwise, every symbol must be unique

                local_hashes[symbol] = elem[1]

                if len(elem) > 2 and (prefix == FUNCTION_PREFIX or prefix == VARIABLE_PREFIX):
                    used_definitions[symbol] = elem[2]
    return (local_hashes, used_definitions)
'''
