#!/usr/bin/env python2

import os
import sys
import logging
import time

tmp_path = "%s/git/versuchung/src"% os.environ["HOME"]
if os.path.exists(tmp_path):
    sys.path.append(tmp_path)

from versuchung.experiment import Experiment
from versuchung.types import String, Bool,Integer,List
from versuchung.files import File, Directory
from versuchung.execute import shell
from versuchung.tex import DatarefDict

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

from incremental_rebuild import IncrementalCompilation
from lib import read_hash_directory

class IncrementalCompilationPlots(Experiment):
    inputs =  {'results': List(IncrementalCompilation(), [])}
    outputs = {'tex': DatarefDict('data.dref')}

    def save(self, path, value):
        self.tex['/'.join(path)] = value
        logging.info("%s = %s", '/'.join(path), value)

    def symlink_name(self):
        return "%s-%s" %(self.title, self.project_name)

    def run(self):
        names = set([x.project_name() for x in self.results])
        assert len(names) == 1, "Different projects are mixed"
        self.project_name = list(names)[0]

        for result in self.results:
            print(result.stats.path)
            records = eval(result.stats.value)
            print result.name, len(records['builds'])
            build_times_all = []
            build_times_headers = []
            build_times_sources = []

            #hash_info = read_hash_directory(result.hashes.path)
            #print(hash_info)

            for build in records['builds']:
                t = build['build-time'] / 1e9

                if build['filename'] == "FRESH_BUILD":
                    print(result.variant_name(), "FB", t)
                    continue
                # Get a float in seconds
                build_times_all.append(t)
                if build['filename'].endswith('.h'):
                    build_times_headers.append(t)
                else:
                    build_times_sources.append(t)
                if "alltypes" in build['filename']:
                    print(result.variant_name(), t)

                #print(build['id'])

            self.save([result.variant_name(), 'rebuild', 'avg'],
                      np.average(build_times_all))
            if build_times_sources:
                self.save([result.variant_name(), 'rebuild', 'sources', 'avg'],
                          np.average(build_times_sources))
            if build_times_headers:
                self.save([result.variant_name(), 'rebuild', 'headers', 'avg'],
                          np.average(build_times_headers))


if __name__ == "__main__":
    experiment = IncrementalCompilationPlots()
    dirname = experiment(sys.argv + ["-s"])
    print(dirname)
