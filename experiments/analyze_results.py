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
        method_stats = defaultdict(lambda: defaultdict(lambda: 0))
        HITS = defaultdict(lambda : {})

        for (project, results) in groupby(x, key=lambda x:x.project_name()):
            times = defaultdict(lambda: dict())

            for result in sorted(results, key=lambda x:x.variant_name()):
                key = [result.variant_name(), 'historical']
                records = eval(result.stats.value)

                # How Many Hits were produced by clang-hash/ccache
                stats = defaultdict(lambda : 0)

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

                    stats['misses/clang-hash'] += build.get('clang-hash-misses',0)
                    stats['hits/clang-hash'] += build.get('clang-hash-hits',0)
                    stats['misses/ccache'] += build.get('ccache-misses',0)
                    stats['hits/ccache'] += build.get('ccache-hits',0)
                    hits = build.get('ccache-hits',0) + build.get('clang-hash-hits',0)
                    stats['hits'] += hits
                    misses = build.get('ccache-misses',0) + build.get('clang-hash-misses',0)
                    if result.metadata['mode'] == "ccache-clang-hash":
                        misses -= (build.get('clang-hash-hits',0) \
                                   + build.get('clang-hash-misses',0))
                    stats['misses'] += misses


                    stats['compiler calls'] = stats['hits'] + stats['misses']

                    build['hits'] = hits
                    build['misses'] = misses
                    build['compiler calls'] = hits + misses
                    HITS[build['commit']][result.metadata['mode']] \
                        =  build

                # Over all builds of an experiment
                def seq(key, seq):
                    self.save(key +["sum"], sum(seq))
                    self.save(key +["count"], len(seq))
                    self.save(key +["avg"],   np.average(seq))

                self.save(key + ["failed"], failed)
                seq(key, build_times)
                for k in stats:
                    self.save(key + [k], stats[k])
                    # Aggregate hits and misses per method
                    method_stats[result.metadata["mode"]][k] += stats[k]

            try:
                x = sorted(times, key=lambda x: times[x]['normal'])
                #print(project, x[0], times[x[0]]['normal'], HITS[x[0]])
                #print(project, x[-1], times[x[-1]]['normal'], HITS[x[-1]])
                #print "------"

                self.save([project, "best commit", "hash"], x[0][0:10])
                for k in ("normal", "ccache", "clang-hash", "ccache-clang-hash"):
                    self.save([project, "best commit", k], times[x[0]][k])
                self.save([project, "best commit", "ratio"], times[x[0]]['clang-hash']/times[x[0]]['normal'])
            except:
                pass

        # Output method statistics
        for method in method_stats:
            for k in method_stats[method]:
                self.save([method, "historical", k], method_stats[method][k])

        for Hash in HITS:
            #if Hash != "fe0a0b5993dfe24e4b3bcf52fa64ff41a444b8f1":
            #    continue
            #print HITS[Hash]
            continue
            if 'clang-hash' not in HITS[Hash]:
                continue
            if HITS[Hash]['clang-hash']['hits'] \
               > HITS[Hash]['ccache-clang-hash']['hits']:
                print Hash, \
                    (HITS[Hash]['clang-hash']['hits'],
                     HITS[Hash]['ccache-clang-hash']['hits'])
                for i in ("clang-hash", "ccache-clang-hash"):
                    print i, HITS[Hash][i]

                print "-----"

if __name__ == "__main__":
    experiment = AnalyzeResults()
    dirname = experiment(sys.argv + ["-s"])
