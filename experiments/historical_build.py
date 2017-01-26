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

class HistoricalCompilation(Experiment, ClangHashHelper):
    inputs = {
        "clang_hash": GitArchive("/home/stettberger/w/clang-hash/"),
        "project": GitArchive("/home/stettberger/w/clang-hash/hash-projects/lua"),
        "mode": String("normal"),
        "commits": Integer(500),
        "jobs": Integer(4),
    }
    outputs = {
        "stats": File("summary.dict"),
    }

    def run(self):
        # Determine the mode
        modes = ('normal', 'ccache', 'clang-hash', 'ccache-clang-hash')
        if not self.mode.value in modes:
            raise RuntimeError("Mode can only be one of: %s"%modes)

        logging.info("Build the Clang-Hash Plugin")
        with self.clang_hash as cl_path:
            shell("cd %s; mkdir build; cd build; cmake .. -DCMAKE_BUILD_TYPE=Release; make -j 4", cl_path)
            shell("strip %s/build/src/*.so", cl_path)

        # Project name
        logging.info("Cloning project... %s", self.project_name())
        self.build_info = {"project-name": self.project_name(),
                           "commit-hash": self.metadata["project-hash"],
                           'builds': []}

        with self.project as src_path:
            (commits, _) = shell("cd %s; git log --oneline --topo-order", src_path)
            commits = [x.split(" ", 1) for x in reversed(commits)]
            commits = commits[-self.commits.value:]

            # First, we redirect all calls to the compiler to our
            # clang hash wrapper
            self.setup_compiler_paths(cl_path)

            while True:
                commit = commits.pop(0)
                logging.info("Build: %s", commit)
                shell("cd %s; git clean -dfx; git reset --hard %s", src_path, commit[0])
                info = {"commit": "FRESH_BUILD"}
                            # Initial build of the given project
                self.call_configure(src_path)
                self.rebuild(src_path, info, True)
                # Did initial commit fail? Try again
                if "failed" in info:
                    continue
                self.build_info["builds"].append(info)
                break

            time = 0
            for commit in commits:
                shell("cd %s; git reset --hard %s", src_path, commit[0])
                info = {"commit": commit[0], "summary": commit[1]}
                if self.project_name() == "lua":
                    self.call_configure(src_path)
                self.rebuild(src_path, info, fail_ok=True)
                self.build_info["builds"].append(info)
                if not info.get("failed"):
                    time += info['build-time'] / 1e9
            logging.info("Rebuild for %d commits takes %f minutes",
                         self.commits.value, time/60.)

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
