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
        hist = defaultdict(lambda: 0)
        for (project, results) in groupby(x, key=lambda x:x.project_name()):
            times = defaultdict(lambda: dict())


            for result in sorted(results, key=lambda x:x.variant_name()):
                key = [result.variant_name(), 'historical']
                records = eval(result.stats.value)

                # How Many Hits were produced by clang-hash/ccache
                hits = 0
                misses = 0
                hash_hits = 0
                if os.path.exists(result.clang_hash_stats.path):
                    hash_hits = result.clang_hash_stats.value.count("H")
                    hash_misses = result.clang_hash_stats.value.count("M")
                    self.save(key + ["hits", "clang-hash"], hash_hits)
                    self.save(key + ["miss", "clang-hash"], hash_misses)
                    hits += hash_hits
                    misses += hash_misses


                if os.path.exists(result.ccache_stats.path):
                    ccache_hits = 0
                    ccache_misses = 0
                    for line in result.ccache_stats.value.split("\n"):
                        if "cache hit" in line:
                            ccache_hits += int(line[line.index(")")+1:].strip())
                        if "cache miss" in line:
                            ccache_misses += int(line[line.index("miss")+4:].strip())

                    self.save(key + ["hits", "ccache"], ccache_hits)
                    self.save(key + ["misses", "ccache"], ccache_misses)
                    hits += ccache_hits
                    misses += (ccache_misses - hash_hits)

                self.save(key + ["hits"], hits)
                self.save(key + ["misses"], misses)

                if os.path.exists(result.ccache_stats.path):
                    text = result.ccache_stats.value

                build_times = []
                failed = 0
                for build in records['builds']:
                    if build.get('failed'):
                        failed += 1
                        continue
                    t = build['build-time']/1e9
                    build_times.append(t)
                    times[build['commit']][result.metadata['mode']] = t
                    hist[int(t)] += 1


                def seq(key, seq):
                    self.save(key +["sum"], sum(seq))
                    self.save(key +["count"], len(seq))
                    self.save(key +["avg"],   np.average(seq))

                self.save(key + ["failed"], failed)
                seq(key, build_times)

            try:
                x = sorted(times, key=lambda x: times[x]['clang-hash']/times[x]['normal'])
                print(project, x[0], times[x[0]]['clang-hash']/times[x[0]]['normal'], times[x[0]])
                # Worst Commit: print(project, x[-1], times[x[-1]]['clang-hash']/times[x[-1]]['normal'], times[x[-1]])


                self.save([project, "best commit", "hash"], x[0][0:10])
                for k in ("normal", "ccache", "clang-hash", "ccache-clang-hash"):
                    self.save([project, "best commit", k], times[x[0]][k])
                self.save([project, "best commit", "ratio"], times[x[0]]['clang-hash']/times[x[0]]['normal'])
            except:
                pass


if __name__ == "__main__":
    experiment = AnalyzeResults()
    dirname = experiment(sys.argv + ["-s"])
