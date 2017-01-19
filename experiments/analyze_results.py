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

class AnalyzeResults(Experiment):
    inputs =  {'results': List(IncrementalCompilation(), [])}
    outputs = {'tex': DatarefDict('data.dref')}

    def save(self, path, value):
        self.tex['/'.join(path)] = value
        logging.info("%s = %s", '/'.join(path), value)

    def run(self):
        self.project_name = ""
        for result in sorted(self.results, key=lambda x: (x.variant_name())):
            records = eval(result.stats.value)
            build_times_all = []
            build_times_headers = []
            build_times_sources = []

            for build in records['builds']:
                t = build['build-time'] / 1e9

                if build['filename'] == "FRESH_BUILD":
                    self.save([result.variant_name(), "fresh build"], t)
                    continue
                # Get a float in seconds
                build_times_all.append(t)
                if build['filename'].endswith('.h'):
                    build_times_headers.append(t)
                else:
                    build_times_sources.append(t)


                #print(build['id'])
            def seq(key, seq):
                self.save(key +["count"], len(seq))
                self.save(key +["avg"],   np.average(seq))

            seq([result.variant_name(), 'rebuild'], build_times_all)
            seq([result.variant_name(), 'rebuild', "sources"], build_times_sources)
            seq([result.variant_name(), 'rebuild', "headers"], build_times_headers)



if __name__ == "__main__":
    experiment = AnalyzeResults()
    dirname = experiment(sys.argv + ["-s"])
