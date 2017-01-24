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
from lib import ClangHashHelper

class IncrementalCompilation(Experiment, ClangHashHelper):
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

    def get_sources(self, path):
        ret = []
        for root, dirnames, filenames in os.walk(path):
            for filename in filenames:
                if filename.endswith(('.h','.c')):
                    ret.append(os.path.join(root, filename))
        if self.project_name() == "musl":
            # We do not touch headers that are external, since they
            # are untouchable.
            ret = [x for x in ret if x.endswith(".c") or "internal" in x ]
        return sorted(ret)

    def touch(self, path):
        if self.touch_only.value:
            os.utime(path, None)
        else:
            with open(path) as fd:
                content = fd.read()
            content = "#line 1\n" + content
            with open(path, "w") as fd:
                fd.write(content)


    def run(self):
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
            sources = list(self.get_sources(src_path))
            nr_files = len(sources)
            logging.info("#files: %d", nr_files)
            self.build_info['file-count'] = nr_files

            # Initial build of the given project
            self.call_configure(src_path)
            info = {"filename": "FRESH_BUILD"}
            self.rebuild(src_path, info)
            self.build_info["builds"].append(info)

            # Iterate over all files
            for fn in sources:
                self.touch(fn)
                info = {"filename": fn}
                self.rebuild(src_path, info)
                self.build_info["builds"].append(info)

        # Output the summary of this build into the statistics file.
        with open(self.stats.path, "w+") as fd:
            fd.write(repr(self.build_info))

    def method_name(self):
        mod = "append"
        if self.metadata['touch-only']:
            mod = "touch"
        return "%s-%s" %(mod, self.metadata['mode'])

    def variant_name(self):
        return "%s-%s"%(self.project_name(), self.method_name())

    def symlink_name(self):
        return "%s-%s"%(self.title, self.variant_name())


if __name__ == "__main__":
    experiment = IncrementalCompilation()
    dirname = experiment(sys.argv)
