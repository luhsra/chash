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
        "clang_hash": GitArchive("/home/stettberger/w/clang-hash/"),
        "project": GitArchive("/home/stettberger/w/clang-hash/hash-projects/musl",
                              shallow=True),
        "touch-only": Bool(False),
        "mode": String("normal"),
        "jobs": Integer(4),
    }
    outputs = {
        "stats": File("summary.dict"),
    }

    def setup_compiler_paths(self, clang_path):
        if self.mode.value == "normal":
            CC = os.path.join(clang_path, "build/wrappers/clang-normal")
        elif self.mode.value == "clang-hash":
            CC = os.path.join(clang_path, "build/wrappers/clang-hash-stop")
        elif self.mode.value == "ccache":
            cache_dir = os.path.join(self.tmp_directory.path, "ccache")
            os.mkdir(cache_dir)
            os.environ["CCACHE_DIR"] = cache_dir
            CC = os.path.join(clang_path, "build/wrappers/clang-ccache")
        else:
            raise RuntimeError("Not a valid mode")

        os.environ['CC'] = CC
        self.CC = CC

    def call_configure(self, path):
        if self.project_name() == "postgresql":
            shell("cd %s; ./configure --enable-depend", path)
        elif self.project_name() == "musl":
            shell("cd %s; ./configure", path)
        else:
            raise RuntimeError("Not a valid project")

    def call_make(self, path):
        return shell("cd %s; make -j %s", path, str(self.jobs.value))

    def get_sources(self, path):
        ret = []
        for root, dirnames, filenames in os.walk(path):
            for filename in filenames:
                if filename.endswith(('.h','.c')):
                    ret.append(os.path.join(root, filename))
        return sorted(ret)

    def touch(self, path):
        if self.touch_only.value:
            os.utime(path, None)
        else:
            with open(path, "a") as fd:
                fd.write("\n")

    def rebuild(self, path, cause):
        info = {'filename': cause}
        self.build_info['builds'].append(info)

        # We export the RUN ID to the clang wrapper script, so it can
        # include it into the object file records

        # Recompile!
        start_time = time.time()
        ret = self.call_make(path)
        end_time = time.time()

        # Call the lines that include the full compiler path
        compiler_calls = len([1 for x in ret[0]
                              if x.startswith(self.CC) and '-c' in x])
        # Account only nano seconds, everywhere
        build_time = int((end_time - start_time) * 1e9)
        info['build-time'] = build_time
        info['compiler-calls'] = compiler_calls
        logging.info("Rebuild done[%s]: %s s; CC() = %d ", cause,
                     build_time / 1e9,
                     compiler_calls)
        return ret

    def run(self):
        self.suspend_on_error = True
        # Determine the mode
        modes = ('normal', 'ccache', 'clang-hash')
        if not self.mode.value in modes:
            raise RuntimeError("Mode can only be one of: %s"%modes)


        logging.info("Build the Clang-Hash Plugin")
        with self.clang_hash as cl_path:
            shell("cd %s; mkdir build; cd build; cmake ..; make -j 4", cl_path)

        # Project name
        logging.info("Cloning project... %s", self.project_name())
        self.build_info = {"project-name": self.project_name(),
                           "commit-hash": self.metadata["project-hash"],
                           'builds': []}
        with self.project as src_path:
            # First, we redirect all calls to the compiler to our
            # clang hash wrapper
            self.setup_compiler_paths(cl_path)

            # Count the number of files
            nr_files = len(list(self.get_sources(src_path)))
            logging.info("#files: %d", nr_files)
            self.build_info['file-count'] = nr_files

            # Initial build of the given project
            self.call_configure(src_path)
            self.rebuild(src_path, "FRESH_BUILD")

            # Iterate over all files
            for fn in self.get_sources(src_path):
                self.touch(fn)
                self.rebuild(src_path, fn)

        # Output the summary of this build into the statistics file.
        with open(self.stats.path, "w+") as fd:
            fd.write(repr(self.build_info))

    def project_name(self):
        return os.path.basename(self.metadata['project-clone-url'])

    def variant_name(self):
        mod = "append"
        if self.touch_only.value:
            mod = "newline"
        return "%s-%s-%s"%(self.project_name(), mod, self.metadata['mode'])

    def symlink_name(self):
        return "%s-%s"%(self.title, self.variant_name())


if __name__ == "__main__":
    experiment = IncrementalCompilation()
    dirname = experiment(sys.argv)
