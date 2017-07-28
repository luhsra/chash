#!/usr/bin/env python

import os
import re
import subprocess
from subprocess import check_output
import datetime
import time
import sys
import fnmatch
# remove unused imports


make_j = 16


#TODO: make paths independent => make project path command line argument!
project_name = "musl"
path_to_project = os.path.abspath("../hash_projects/musl")
clanghash_wrapper = os.path.abspath("build/wrappers/clang")
build_info_file_path = os.path.abspath("build/muslHashes/buildInfo_%s_with_stop.info" % project_name)


def get_source_files(directory):
    for root, dirnames, filenames in os.walk(directory):
        for filename in filenames:
            if filename.endswith(('.h','.c')):
                yield os.path.join(root, filename)

def touch(filename):
    with open(filename, 'a'):
        os.utime(filename, None)#TODO: evtl None als 2. param

def git_checkout(commit_id):
    """checkout commit with commit_id"""
    os.chdir(path_to_project)
    subprocess.call(["git", "checkout", "-f", "-q", commit_id])

def make_clean():
    subprocess.call(["make", "clean"])

def git_clean():
    subprocess.call(["git", "clean", "-f", "-q", "-x"])

def call_configure():
    """
    TODO: rename this function to "init_build_environment" or sth like that and replace it for other projects. also think about calling call_make here (for first build).
    """
    subprocess.call(["./configure"])

def call_make():
    """this function builds the project (e.g. calls make)
    TODO: replace this method for other projects; or move to class and then replace that class.
    """
    print "calling make -j%d" % make_j
    p = subprocess.Popen(["make", "-j%d" % make_j], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    retcode = p.wait()
    print ">>>>>>>>>>"
    print out
    print err
    print "<<<<<<<<<<"


################################################################################


print "Starting at %s" % datetime.datetime.now()

filecount = 0
for root, dirnames, filenames in os.walk(path_to_project):
    for filename in fnmatch.filter(filenames, '*.c'):
        filecount += 1        

print "#files: %d" % filecount


os.environ['CC'] = clanghash_wrapper
os.environ['PROJECT'] = project_name
os.environ['STOP_IF_SAME_HASH'] = '1'

#reset to latest version
git_checkout("master")
print "calling make clean and git clean"
make_clean()
git_clean()
call_configure()

build_info = {}
commit_id = check_output(["git", "log", "-1", "--pretty='%H'"])  # get current commit hash
build_info['commit-hash'] = commit_id
os.environ['COMMIT_HASH'] = commit_id

print "starting initial build at %s" % datetime.datetime.now() #TODO: evtl. refactor duplicate code
run_id = 0  # for collecting the data later
os.environ['RUN_ID'] = "%d" % run_id
build_info[run_id] = {}
current_build_info = build_info[run_id]
current_build_info['filename'] = 'FRESH_BUILD'

start_time = time.time()
call_make()
build_time = int((time.time() - start_time) * 1e9) * make_j # nano * nr of threads
current_build_info['build-time'] = int((time.time() - start_time) * 1e9) * make_j # nano * nr of threads
print "finished initial build at %s, duration: %s" % (datetime.datetime.now(), build_time)

"""
TODO:
for file in dir:
    touch file/insert NL
    make
    collect data (this has to be done in clang.in?):
        times: parse, compilation, hashing
        #files changed/#files w/ same hashes
"""
for filename in get_source_files(path_to_project):
    run_id += 1
    os.environ['RUN_ID'] = "%d" % run_id
    print "current id: %d, current file: %s" % (run_id, filename)
    
    build_info[run_id] = {}
    current_build_info = build_info[run_id]
    current_build_info['filename'] = filename

    touch(filename) #TODO: use abs filepath?
    
    print "starting build at %s" % datetime.datetime.now() #TODO: evtl. refactor duplicate code
    start_time = time.time()
    call_make()
    current_build_info['build-time'] = int((time.time() - start_time) * 1e9) * make_j # nano * nr of threads
    print "finished commit %s at %s" % (commit_id, datetime.datetime.now())



f = open(build_info_file_path, 'a')
f.write(repr(build_info) + "\n")
f.close()
print "Finished at %s" % datetime.datetime.now()

