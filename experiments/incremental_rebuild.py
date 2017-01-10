#!/usr/bin/env python2

import os
import sys
import logging
import time

tmp_path = "%s/git/versuchung/src"% os.environ["HOME"]
if os.path.exists(tmp_path):
    sys.path.append(tmp_path)

from versuchung.experiment import Experiment
from versuchung.types import String, Bool,Integer
from versuchung.files import File, Directory
from versuchung.archives import GitArchive
from versuchung.execute import shell
from versuchung.tex import DatarefDict

class IncrementalCompilation(Experiment):
    inputs = {
        "clang_hash": GitArchive("/home/stettberger/git/studenten/clang-hash/"),
        "project": GitArchive("/home/stettberger/git/studenten/clang-hash/hash-projects/musl"),
        "touch-only": Bool(True),
        "mode": String("normal"),
        "jobs": Integer(4),
    }
    outputs = {
        "hashes": Directory("hashes"),
        "stats": File("summary.dict"),
    }

    def setup_compiler_paths(self, clang):
        os.environ['CC'] = clang
        if self.mode.value == "normal":
            os.environ['STOP_IF_SAME_HASH'] = '0'
        elif self.mode.value == "clang-hash":
            os.environ['STOP_IF_SAME_HASH'] = '1'
        else:
            sys.exit("Invalid operation mode")

    def call_configure(self, path):
        shell("cd %s; ./configure", path)

    def call_make(self, path):
        shell("cd %s; make -j %s", path, str(self.jobs.value))

    def get_sources(self, path):
        for root, dirnames, filenames in os.walk(path):
            for filename in filenames:
                if filename.endswith(('.h','.c')):
                    yield os.path.join(root, filename)

    def touch(self, path):
        if self.touch_only.value:
            os.utime(path, None)
        else:
            with open(path, "a") as fd:
                fd.write("\n")

    def rebuild(self, path, cause):
        run_id = len(self.build_info)
        self.build_info[run_id] = {}
        self.build_info[run_id]['filename'] = cause
        os.environ['RUN_ID'] = "%d" % run_id

        # Recompile!
        start_time = time.time()
        self.call_make(path)
        end_time = time.time()

        # Account only nano seconds, everywhere
        build_time = int((end_time - start_time) * 1e9) * self.jobs.value
        self.build_info[run_id]['build-time'] = build_time
        logging.info("Rebuild done[%s]: %s s", cause,
                     build_time / 1e9)


    def run(self):
        # Determine the mode
        modes = ('normal', 'clang-hash')
        if not self.mode.value in modes:
            raise RuntimeError("Mode can only be one of: %s"%modes)


        logging.info("Build the Clang-Hash Plugin")
        with self.clang_hash as cl_path:
            shell("cd %s; mkdir build; cd build; cmake ..; make -j 4", cl_path)
            clanghash_wrapper = os.path.join(cl_path, "build/wrappers/clang")

        # Project name
        self.project_name = os.path.basename(self.project.checkout_url())
        os.environ["PROJECT"] = self.project_name
        logging.info("Cloning project... %s", self.project_name)
        self.build_info = {"project-name": self.project_name,
                      "commit-hash": self.metadata["project-hash"]}
        with self.project as src_path:
            # First, we redirect all calls to the compiler to our
            # clang hash wrapper
            self.setup_compiler_paths(clanghash_wrapper)
            os.environ["COMMIT_HASH"] = self.metadata["project-hash"]
            os.environ["CLANG_HASH_OUTPUT_DIR"] = self.hashes.path

            nr_files = len(list(self.get_sources(src_path)))
            logging.info("#files: %d", nr_files)

            # Initial build of the given project
            self.call_configure(src_path)
            self.rebuild(src_path, "FRESH_BUILD")

            # Iterate over all files
            for fn in self.get_sources(src_path):
                self.touch(fn)
                self.rebuild(src_path, fn)
                break



        # Output the summary of this build into the statistics file.
        with open(self.stats.path, "w+") as fd:
            fd.write(repr(self.build_info))

if __name__ == "__main__":
    experiment = IncrementalCompilation()
    dirname = experiment(sys.argv + ["-s"])
    print(dirname)
