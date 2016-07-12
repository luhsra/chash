#!/usr/bin/env python

import fnmatch
import os
import sys


#TODO: alle generierten/gemessenen werte einlesen und vergleichen
# wsl am besten einfach beim neuesten commit anfangen und dann rueckwaerts vorgehen

def getListOfFiles(directory):
    for root, dirnames, filenames in os.walk(directory):
        for filename in fnmatch.filter(filenames, '*.info'):
            yield os.path.join(root, filename)


def printHashInfo(message, prevRecord, record, isError=True):
    print "%s: file %s, commits %s to %s : %s" % ("ERROR" if isError else "INFO", record['filename'], prevRecord['commit-hash'], record['commit-hash'], message)
 

def validateHashes(records):

    #TODO: sort files by start-time

    iterRecords = iter(records)
    prevRecord = next(iterRecords)
    filename = prevRecord['filename']
    if 'ast-hash' not in prevRecord.keys():
        print "MISSING: no ast-hash in records for file " + filename
        return
    for record in records:
        if prevRecord['ast-hash'] == record['ast-hash']:
            if prevRecord['object-hash'] != record['object-hash']:
                printHashInfo("ast hashes same, object hashes differ", prevRecord, record)
        if prevRecord['object-hash'] != record['object-hash']:
            if prevRecord['ast-hash'] == record['ast-hash']:
                printHashInfo("object hashes differ, ast hashes same", prevRecord, record)
        elif prevRecord['ast-hash'] != record['ast-hash']:
            printHashInfo("ast hashes differ, object hashes same", prevRecord, record, False)

        prevRecord = record


################################################################################

if (len(sys.argv) > 1):
    pathToRecords = sys.argv[1]
    records = []
    for filename in getListOfFiles(pathToRecords):
        records = [eval(line) for line in open(filename)]
        validateHashes(records)
else:
    print "Missing path to record files.\nUsage:\n\t%s pathToRecords" % sys.argv[0]

