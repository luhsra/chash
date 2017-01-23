#!/usr/bin/env python2

import os
import re
import sys
import logging
import time
from itertools import groupby
from collections import defaultdict

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
from historical_build  import HistoricalCompilation


class AnalyzeResults(Experiment):
    inputs =  {
        'incremental': List(IncrementalCompilation(), []),
        'historical': List(HistoricalCompilation(), []),
    }
    outputs = {'tex': DatarefDict('data.dref')}

    def save(self, path, value):
        self.tex['/'.join(path)] = value
        logging.info("%s = %s", '/'.join(path), value)


    def run(self):
        self.project_name = ""
        x = sorted(self.incremental, key=lambda x:x.project_name())
        for (project, results) in groupby(x, key=lambda x:x.project_name()):
            times = defaultdict(lambda: dict())

            for result in sorted(results, key=lambda x:x.variant_name()):
            	records = eval(result.stats.value)
            	build_times_all = []
            	build_times_headers = []
            	build_times_sources = []

            	for build in records['builds']:
            	    t = build['build-time'] / 1e9

                    fn = re.sub(".*?/project/", "", build['filename'])
                    times[fn][result.method_name()] = t


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

            def score(x):
                return times[x]['touch-clang-hash']/times[x]['touch-normal']
            x = sorted(times, key=score)
            best = x[0]
            print(project, best, score(best), times[best])

        ################################################################
        # Historical Build Times
        ################################################################
        x = sorted(self.historical, key=lambda x:x.project_name())
        for (project, results) in groupby(x, key=lambda x:x.project_name()):
            times = defaultdict(lambda: dict())

            for result in sorted(results, key=lambda x:x.variant_name()):
                records = eval(result.stats.value)

                build_times = []
                for build in records['builds']:
                    t = build['build-time']/1e9
                    build_times.append(t)
                    times[build['commit']][result.metadata['mode']] = t


                def seq(key, seq):
                    self.save(key +["sum"], sum(seq))
                    self.save(key +["count"], len(seq))
                    self.save(key +["avg"],   np.average(seq))

                seq([result.variant_name(), 'historical'], build_times)

            try:
                x = sorted(times, key=lambda x: times[x]['clang-hash']/times[x]['normal'])
                print(project, x[0], times[x[0]]['clang-hash']/times[x[0]]['normal'], times[x[0]])
            except:
                pass




if __name__ == "__main__":
    experiment = AnalyzeResults()
    dirname = experiment(sys.argv + ["-s"])
