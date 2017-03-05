#!/usr/bin/env python2

import os
import sys
import logging
import time
from collections import defaultdict
import numpy as np


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

class TimingInternal(Experiment, ClangHashHelper):
    inputs = {
        "clang_hash": GitArchive("/home/stettberger/w/clang-hash/"),
        "project": GitArchive("/home/stettberger/w/clang-hash/hash-projects/lua",
                              shallow=True),
        "cflags": String(""),
        "jobs": Integer(4),
        "mode": String("normal"), # Unchangable
    }
    outputs = {
        "stats": File("summary.dict"),
        'tex': DatarefDict('data.dref'),
    }

    def save(self, path, value):
        self.tex['/'.join(path)] = value
        logging.info("%s = %s", '/'.join(path), value)

    def run(self):

        with self.clang_hash as cl_path:
            logging.info("Cloning clang hash...")

        logging.info("Cloning project... %s", self.project_name())

        # First, we redirect all calls to the compiler to our
        # gcc wrapper
        CC = os.path.join(cl_path, "wrappers/gcc-time")
        os.environ["CC"] = CC
        os.environ["TIMING_REPORT"] = self.stats.path
        os.environ["CHASH_EXTRA_FLAGS"] = self.cflags.value
        with self.project as src_path:
            info = {}
            self.call_configure(src_path)
            self.rebuild(src_path, info, True)

        collect = defaultdict(list)
        compiler_calls = 0
        with open(self.stats.path) as fd:
            for line in fd.readlines():
                data = eval(line)
                if "name" in data:
                    compiler_calls += 1
                for key in data:
                    if type(data[key]) is float:
                        collect[key].append(data[key])
        self.save([self.project_name(), "phase", self.cflags.value, "count"],
                       compiler_calls)
        for phase in collect:
            if phase in ("preprocessing", "parser (global)", "phase opt and generate"):
                self.save([self.project_name(), "phase", phase, self.cflags.value],
                          np.average(collect[phase]))

    def symlink_name(self):
        return "%s-%s%s"%(self.title,
                           self.project_name(),
                           self.cflags.value)


if __name__ == "__main__":
    experiment = TimingInternal()
    dirname = experiment(sys.argv)
