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
        "ccache_stats": File("ccache.stats"),
        "clang_hash_stats": File("clang-hash.stats"),
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
            shell("cd %s; git clean -dfx", src_path)
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
            shell("strip %s/build/src/*.so", cl_path)

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

                # Bash initial commit
                if commit[0] == "726f63884db0132f01745f1fb4465e6621088ccf":
                    continue


                info = {"commit": commit[0],
                        "parent": commit[1],
                        "summary": commit[2]}

                # First, we build the parent. In a total linear
                # history, this is a NOP. Otherwise, we try to reset
                # to the actual parent, and rebuild the project. This
                # may fail, since the current commit might fix this.
                ret = self.build_parent(commit[0], from_scratch = last_failed)
                info['parent-failed'] = ret

                # Change to the ACTUAL commit. Call reconfigure, and
                # then go on building the commit.
                shell("cd %s; git reset --hard %s", src_path, commit[0])
                self.call_reconfigure(src_path)
                self.rebuild(src_path, info, fail_ok=True)

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
