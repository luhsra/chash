#!/usr/bin/env python2

import os
import sys
import logging
import subprocess
import time

from copy import deepcopy

tmp_path = "%s/git/versuchung/src"% os.environ["HOME"]
if os.path.exists(tmp_path):
    sys.path.append(tmp_path)

from versuchung.experiment import Experiment
from versuchung.types import String, Bool,Integer
from versuchung.files import File, Directory
from versuchung.archives import GitArchive
from versuchung.execute import shell
from versuchung.tex import DatarefDict
from lib import ClangHashHelper
from lib import read_hash_directory

class HistoricalCompilation(Experiment, ClangHashHelper):
    inputs = {
        "clang_hash": GitArchive("/home/cip/2015/yb90ifym/clang-hash/"),
        "project": GitArchive("/home/cip/2015/yb90ifym/lua"),
        "mode": String("normal"),
        "commits": Integer(500), # was 500
        "jobs": Integer(1), # was 4
    }
    outputs = {
        "stats": File("summary.dict"),
        "ccache_stats": File("ccache.stats"),
        "clang_hash_log": File("clang-hash.log"),
    }

    def build_parent(self, commit, from_scratch = False):
        def eq_hash(a, b):
            if len(a) == 0 or len(b) == 0:
                return
            if len(a) > len(b):
                return a.startswith(b)
            else:
                return b.startswith(a)

        src_path = self.project.path

        if from_scratch:
            shell("cd %s; git clean -dfx -e '*.hash' -e '*.hash.copy'", src_path)
            logging.info("Parent [%s^]: clean build", commit)
            shell("cd %s; git reset --hard %s^", src_path, commit)
            info = {"commit": commit + "^"}
            self.call_configure(src_path)
            self.rebuild(src_path, info, True)
            # Did initial commit fail? Try again
            if info.get("failed"):
                logging.info("Parent[%s^]: failed", commit)
                return False
            return True
        else:
            (lines, _) = shell("cd %s; git rev-parse %s^",
                               src_path, commit)
            parent_revision = lines[0].strip()
            if self.current_revision and eq_hash(self.current_revision, parent_revision):
                logging.info("Parent[%s^]: resuse good parent", commit)
                return True
            else:
                logging.info("Parent[%s^]: resuse similar build directory", commit)
                shell("cd %s; git reset --hard %s^", src_path, commit)
                info = {"commit": commit +"^"}
                self.call_reconfigure(src_path)
                self.rebuild(src_path, info, True)
                # Did initial commit fail? Try again
                if info.get("failed"):
                    return self.build_parent(commit, from_scratch=True)
                return True


    def run(self):
        # Determine the mode
        modes = ('normal', 'ccache', 'clang-hash', 'ccache-clang-hash')
        if not self.mode.value in modes:
            raise RuntimeError("Mode can only be one of: %s"%modes)

        logging.info("Build the Clang-Hash Plugin")
        with self.clang_hash as cl_path:
            shell("cd %s; mkdir build; cd build; cmake .. -DCMAKE_BUILD_TYPE=Release; make -j 4", cl_path)
            shell("strip %s/build/clang-plugin/*.so", cl_path)

        # Project name
        logging.info("Cloning project... %s", self.project_name())
        self.build_info = {"project-name": self.project_name(),
                           "commit-hash": self.metadata["project-hash"],
                           'builds': []}

        with self.project as src_path:
            (commits, _) = shell("cd %s; git log --no-merges --oneline --topo-order --format='%%H %%P %%s'", src_path)
            # [0] is hash. [1] is parent, [2] rest
            commits = [x.split(" ", 2) for x in reversed(commits)]
            commits = commits[-self.commits.value:]

            self.current_revision = None

            # First, we redirect all calls to the compiler to our
            # clang hash wrapper
            self.setup_compiler_paths(cl_path)

            time = 0
            last_failed = True


            nr_of_commits = len(commits)
            original_commits = commits[:]
            occurred_errors = {} # map commit -> [error strings]


            def gather_local_hashes(src_path):
                remove_keys = ['project', 'return-code', 'start-time',
                               'run_id', 'compile-duration',
                               'processed-bytes', 'hash-duration',
                               'hash-start-time', 'object-file-size'] # TODO: ofile-size useful?
                hashes = read_hash_directory(src_path, remove_keys)    
                local_hashes = {}
                for entry in hashes:
                    element_hashes = entry['element-hashes']
                    for element in element_hashes:
                        local_hashes[element[0]] = element[1]

                return local_hashes 

            def gather_global_hashes(local_hashes, occurred_errors):
                global_hashes = {}
                for symbol in local_hashes:
                    symbol = symbol.split(':')[1] # Remove the prefix ('function:' etc.)
                    try:
                        shell("cd %s; %s/clang-hash-global --definition %s", src_path, self.inputs.clang_hash.path, symbol)
                    except Exception as e:
                        occurred_errors[commit[0]] = e
                        # don't raise exception

                return global_hashes

            def add_additional_commit_info_to(info):
                gitshow = subprocess.Popen(["git", "show"], stdout=subprocess.PIPE)
                dstat_out = subprocess.check_output(('diffstat'), stdin=gitshow.stdout)
                gitshow.wait()

                lines = dstat_out.split('\n')
                index = -1
                while lines[index] == '':
                    index -= 1
                last_line = lines[index]
                changedInsertionsDeletions = [int(s) for s in last_line.split() if s.isdigit()]

                if "insertion" in last_line:
                    info['insertions'] = changedInsertionsDeletions[1]
                    if "deletion" in last_line:
                        info['deletions'] = changedInsertionsDeletions[2]
                elif "deletion" in last_line:
                    info['deletions'] = changedInsertionsDeletions[1]

                # Get changed files
                changed_files = {}
                for line in lines:
                    if '|' in line:
                        elems = line.split()

                        assert elems[1] == '|'

                        filename = elems[0]
                        nr_of_changes = int(elems[2])
                        changed_files[filename] = nr_of_changes

                assert len(changed_files) == changedInsertionsDeletions[0]
                info['changes'] = changed_files


            while commits:
                # Search for a child of the current revision
                commit = None
                if self.current_revision:
                    for idx in range(0, len(commits)):
                        if commits[idx][1] == self.current_revision:
                            commit = commits[idx]
                            del commits[idx]
                            break
                # No Child found -> Take the first one.
                if not commit:
                    commit = commits.pop(0)

                info = {"commit": commit[0],
                        "parent": commit[1],
                        "summary": commit[2]}

                # First, we build the parent. In a total linear
                # history, this is a NOP. Otherwise, we try to reset
                # to the actual parent, and rebuild the project. This
                # may fail, since the current commit might fix this.
                ret = self.build_parent(commit[0], from_scratch = last_failed)
                info['parent-ok'] = ret

                parent_info = {}
                add_additional_commit_info_to(parent_info)
                info['parent-info'] = parent_info

                # Gather hashes of parent
                parent_local_hashes = gather_local_hashes(src_path)
                parent_global_hashes = gather_global_hashes(parent_local_hashes,
                                                            occurred_errors)

                #info['parent-local-hashes'] = parent_local_hashes
                #info['parent-global-hashes'] = parent_global_hashes

                # Change to the ACTUAL commit.
                shell("cd %s; git reset --hard %s", src_path, commit[0])


                
                add_additional_commit_info_to(info)
               
                # Call reconfigure, and then go on building the commit.
                self.call_reconfigure(src_path)
                if os.path.exists("/tmp/clang-hash.log"):
                    os.unlink("/tmp/clang-hash.log")

                # Rebuild and Measure
                self.rebuild(src_path, info, fail_ok=True)
                
                # Don't need those atm
                del info['clang-hash-hits']
                del info['clang-hash-misses']

                # Gather hashes
                local_hashes = gather_local_hashes(src_path)
                global_hashes = gather_global_hashes(local_hashes,
                                                     occurred_errors)

                #info['local-hashes'] = local_hashes
                #info['global-hashes'] = global_hashes


                # Compare hashes/search for changed hashes
                # The parent's global hashes are copied to find removed symbols
                changed_symbols ={} 
                parent_hashes = deepcopy(parent_global_hashes)
                for symbol, global_hash in global_hashes.iteritems():
                    parent_global_hash = parent_hashes.pop(symbol, None)
                        
                    if global_hash != parent_global_hash:
                        # Store it as [before, after]
                        changed_symbols[symbol] = [parent_global_hash, global_hash]

                # Add removed symbols
                for symbol, parent_global_hash in parent_hashes.iteritems():
                    changed_symbols[symbol] = [parent_global_hash, None]



                # Compare hashes/search for changed hashes
                # The parent's global hashes are copied to find removed symbols
                local_changed_symbols ={} 
                parent_hashes = deepcopy(parent_local_hashes)
                for symbol, local_hash in local_hashes.iteritems():
                    parent_local_hash = parent_hashes.pop(symbol, None)
                        
                    if local_hash != parent_local_hash:
                        # Store it as [before, after]
                        local_changed_symbols[symbol] = [parent_local_hash, local_hash]

                # Add removed symbols
                for symbol, parent_local_hash in parent_hashes.iteritems():
                    local_changed_symbols[symbol] = [parent_local_hash, None]



                info['changed-symbols'] = changed_symbols
                #info['local-changed-symbols'] = local_changed_symbols
                info['local-changed-sym-count'] = len(local_changed_symbols)


                # TODO: add more analysis
                # TODO: for each changed local hash, the symbol's global hash should also change...
                # check every symbol for changed global hash\
                # also check the commits, if the correct ones are used... 









               
                if os.path.exists("/tmp/clang-hash.log") and not info.get("failed"):
                    with open("/tmp/clang-hash.log") as fd:
                        self.clang_hash_log.value += fd.read()

                self.build_info["builds"].append(info)
                if not info.get("failed"):
                    time += info['build-time'] / 1e9
                    # Build was good. Remember that.
                    self.current_revision = commit[0]
                    last_failed = False
                else:
                    self.current_revision = None
                    last_failed = True

            logging.info("Rebuild for %d commits takes %f minutes",
                         self.commits.value, time/60.)

            print "\n\noccurred errors:\n"
            print occurred_errors
            print "\n\nchanged symbols:\n"
            print changed_symbols
            print "\n\nlocal changed symbols:\n"
            print local_changed_symbols
            print "\n\n\n"

            if len(changed_symbols) or len(local_changed_symbols):
                print "!!! success: found one !!!"

        # Output the summary of this build into the statistics file.
        with open(self.stats.path, "w+") as fd:
            fd.write(repr(self.build_info))



    def variant_name(self):
        return "%s-%s"%(self.project_name(), self.metadata['mode'])

    def symlink_name(self):
        return "%s-%s"%(self.title, self.variant_name())


if __name__ == "__main__":
    experiment = HistoricalCompilation()
    dirname = experiment(sys.argv)
