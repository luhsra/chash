#!/usr/bin/env python2

import os
import sys
import logging
import subprocess
import time

from copy import deepcopy

tmp_path = "%s/git/versuchung/src"% os.environ["HOME"]
if os.path.exists(tmp_path):
    sys.path.append(tmp_path)

from versuchung.experiment import Experiment
from versuchung.types import String, Integer
from versuchung.files import File, Directory
from versuchung.archives import GitArchive
from versuchung.execute import shell
from versuchung.tex import DatarefDict

from clang_hash_global import get_global_hash # get_global_hash(symbol, global_hashes, local_hashes, used_definitions)


import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt


class HistoricalCompilationGlobalEvaluation(Experiment):
    inputs = {
        "clang_hash": GitArchive("/home/cip/2015/yb90ifym/clang-hash/"),
        "project": GitArchive("/home/cip/2015/yb90ifym/clang-hash/hash-projects/lua"),
        "commits": Integer(4744),
        "jobs": Integer(1), # was 4
        "dataset": Directory("/home/cip/2015/yb90ifym/clang-hash/experiments/HistoricalCompilation-4e7c977077afea3d2ad77aeefe3b472c"), # full lua
        "hot_threshold_percentage": Integer(10), # minimal change percentage for commit to be classified as "hot"
    }
    outputs = {
        "stats": File("summary.dict"),
        "eval_data": File("eval.txt"),
        "hot_commits_histo": File("global_hot_commits.pdf"),
    }


    def project_name(self):
        return os.path.basename(self.metadata['project-clone-url'])


    def run(self):
        # Project name
        logging.info("Cloning project... %s", self.project_name())
        self.build_info = {"project-name": self.project_name(),
                           "commit-hash": self.metadata["project-hash"],
                           'builds': []}

        with self.project as src_path:
            time = 0
            
            os.chdir(self.dataset.path)
            
            # Read summary file from data collection run
            commits = None
            with open("summary.dict") as sf:
                summary = eval(sf.read())
                commits = summary['builds']


            def read_chash_data(commit):
                element_hashes = []
                try:
                    with open(commit, 'r') as cf:
                        commit_data = eval(cf.read())
                        for ofile_data in commit_data:
                            element_hashes.extend(ofile_data['element-hashes'])
                except:
                    pass

                return element_hashes



            stats = {
                'data-empty': set(), # commits with empty info files, e.g. failed to be collected, (first n commits -> missing makefile o.a.)
                'commits': {},
                'elements': {}, # symbol -> how often did this symbol change
            }




            total_changed_globals = 0 # How often was any global changed throughout the history?
            total_changed_records = 0 # How often was any record changed throughout the history?
            total_changed_static_funcs = 0 # How often was any static function changed throughout the history?
            total_changed_functions = 0 # without static functions

            total_insdel_globals = 0 # How often was any global introduced/removed throughout the history?
            total_insdel_records = 0 # How often was any record introduced/removed throughout the history?
            total_insdel_static_funcs = 0 # How often was any static function introduced/removed throughout the history?
            total_insdel_functions = 0 # without static functions

            # in-degree: how many SLOs depend on E?
            # out-degree: how many SLOs does E depend on?
            in_degrees = {} # indegree -> nr of elements with that indegree
            out_degrees = {} # outdegree -> nr of elements wirh that outdegree
            max_in_degree = (None, 0) # (element, degree)
            max_out_degree = (None, 0) # (element, degree)

            prev_commit = None
            prev_hashes = None
            prev_used_definitions = None
            prev_global_hashes = None
            counter = 1
            for info in commits:
                print "\n%d/%d" % (counter, len(commits))
                counter += 1
                commit = info['commit']
                parent = info['parent']
 
                if not parent: # first commit has no parent
                    print "No parent"
                    continue

                commit_data = read_chash_data(commit)
                if not commit_data:
                    # If the data does not exist, note and skip
                    #print "Data empty"
                    stats['data-empty'].add(commit)
                    continue

                local_hashes = {}
                used_definitions = {}

                # just 4 testing:
                for element in commit_data:
                    name = element[0]
                    if name.startswith('static function:') or name.startswith('function:'):
                        name = element[0].split(':')[1]
                    local_hashes[name] = element[1]
                    try:
                        used_definitions[name] = set()
                        for used_def in element[2]:
                            if used_def.startswith('static function:') or used_def.startswith('function:'):
                                used_definitions[name].add(used_def.split(':')[1])
                    except:
                        pass

                # prev:
                #for element in commit_data:
                #    local_hashes[element[0]] = element[1]
                #    try:
                #        used_definitions[element[0]] = element[2]
                #    except:
                #        pass


                parent_hashes = {}
                parent_global_hashes = {}
                parent_used_definitions = {}
                if parent == prev_commit and prev_global_hashes and prev_used_definitions and prev_hashes:
                    #print "Reuse prev_commit"
                    parent_hashes = prev_hashes
                    parent_used_definitions = prev_used_definitions
                    parent_global_hashes = prev_global_hashes
                else:
                    #print "Cannot reuse prev_commit"
                    parent_data = read_chash_data(parent)

                    # just 4 testing:
                    for element in parent_data:
                        name = element[0]
                        if name.startswith('static function:') or name.startswith('function:'):
                            name = element[0].split(':')[1]
                        parent_hashes[name] = element[1]
                        try:
                            parent_used_definitions[name] = set()
                            for used_def in element[2]:
                                if used_def.startswith('static function:') or used_def.startswith('function:'):
                                    parent_used_definitions[name].add(used_def.split(':')[1])
                        except:
                            pass



                    # prev:
                    #for element in parent_data:
                    #    parent_hashes[element[0]] = element[1]
                    #    try:
                    #        parent_used_definitions[element[0]] = element[2]
                    #    except:
                    #        pass

                                
                if not parent_hashes:
                    # If the data does not exist, note and skip
                    stats['data-empty'].add(commit)
                    
                    # Save data for reuse
                    prev_commit = commit
                    prev_hashes = local_hashes
                    prev_used_definitions = used_definitions
                    continue

                ##########################
                # GLOBAL HASH EVALUATION #
                ##########################
                
                commit_stats = {
                    'element-count' : len(local_hashes),
                    'changed-elements' : [],
                }


                elements = set(local_hashes.keys())
                parent_elements = set(parent_hashes.keys())
              


                # calculate in- and out-degree
                # reverse used_definitions
                out_use_defs = { s:0 for s in used_definitions.keys() } # element -> nr of depending elements
                for element in elements:
                   for el in used_definitions[element]:
                        try:
                            out_use_defs[el] += 1
                        except:
                            pass
                    

                for element in elements:
                    out_degree = len(used_definitions[element])
                    in_degree = out_use_defs[element] 
                    
                    if in_degree > max_in_degree[1]:
                        max_in_degree = (element, in_degree)
                    if out_degree > max_out_degree[1]:
                        max_out_degree = (element, out_degree)

                    if in_degree not in in_degrees:
                        in_degrees[in_degree] = 0
                    in_degrees[in_degree] += 1

                    if out_degree not in out_degrees:
                        out_degrees[out_degree] = 0
                    out_degrees[out_degree] += 1

 
                commit_stats['changed-elements'] = elements ^ parent_elements # elements either added or removed

                for element in commit_stats['changed-elements']:
                     if element.startswith('record:'): # do this here to get only insertions and deletions
                         total_insdel_records += 1
                     elif element.startswith('variable:') or element.startswith('static variable:'):
                         total_insdel_globals += 1
                     elif element.startswith('static function:'):
                         total_insdel_static_funcs += 1
                     else:
                         total_insdel_functions += 1


                # Compare hashes
                common_elements = elements & parent_elements
                
                global_hashes = {}
                for element in common_elements:
                    global_hash = get_global_hash(element, global_hashes, local_hashes, used_definitions)
                    parent_global_hash = get_global_hash(element, parent_global_hashes, parent_hashes, parent_used_definitions)
                    if global_hash != parent_global_hash:
                        commit_stats['changed-elements'].add(element)
                        if element.startswith('record:'): # do this here to ignore insertions and deletions
                            total_changed_records += 1
                        elif element.startswith('variable:') or element.startswith('static variable:'):
                            total_changed_globals += 1
                        elif element.startswith('static function:'):
                            total_changed_static_funcs += 1
                        else:
                            total_changed_functions += 1

                commit_stats['changed-element-count'] = len(commit_stats['changed-elements']);
                stats['commits'][commit] = commit_stats


                # Count how often each element was changed over the whole history
                for element in commit_stats['changed-elements']:
                    if element not in stats['elements']:
                        stats['elements'][element] = 0;
                    stats['elements'][element] += 1


                # Save data for reuse
                prev_commit = commit
                prev_hashes = local_hashes
                prev_used_definitions = used_definitions
                prev_global_hashes = global_hashes

            self.build_info['stats'] = stats

        #in_degrees = {} # indegree -> nr of elements with that indegree
        #out_degrees = {} # outdegree -> nr of elements wirh that outdegree
        #max_in_degree = (None, 0) # (element, degree)
        #max_out_degree = (None, 0) # (element, degree)
        summed_in_degrees = sum([k*v for k,v in in_degrees.iteritems()])
        nr_of_elements = sum(in_degrees.values())
        avg_in_degree = summed_in_degrees/float(nr_of_elements)
        avg_out_degree = sum([k*v for k,v in out_degrees.iteritems()])/float(sum(out_degrees.values()))


        eval_info = {
            'nr-of-commits' : len(commits),
            'change-percentage' : {}, # change percentage -> nr of commits with change < percentage
            'hot-commits': {},
            'total-changed-globals': total_changed_globals,
            'total-changed-records': total_changed_records,
            'total-changed-static-funcs': total_changed_static_funcs,
            'total-changed-functions': total_changed_functions,
            'total-insdel-globals': total_insdel_globals,
            'total-insdel-records': total_insdel_records,
            'total-insdel-static-funcs': total_insdel_static_funcs,
            'total-insdel-functions': total_insdel_functions,
            'max_in_degree': max_in_degree,
            'max_out_degree': max_out_degree,
            'avg_in_degree': avg_in_degree, 
            'avg_out_degree': avg_out_degree,
        }

        # Get most changed elements
        eval_info['most-changed-elements'] = {k:v for k,v in stats['elements'].iteritems() if v > 1000} # arbitrary value (about 20% of commits)
        
        # Calc average nr and percentage of (changed) symbols per commit
        summed_avg_change_percentage = 0
        summed_changed_elements = 0
        summed_total_elements = 0
        commits = self.build_info['stats']['commits']
        for commit in commits:
            commit_stat = commits[commit]
            change_percentage = len(commit_stat['changed-elements'])/float(commit_stat['element-count'])
            summed_avg_change_percentage += change_percentage

            summed_changed_elements += len(commit_stat['changed-elements'])
            summed_total_elements += commit_stat['element-count']

            percentage = int(round(change_percentage * 100))
            if percentage not in eval_info['change-percentage']:
                eval_info['change-percentage'][percentage] = 0
            eval_info['change-percentage'][percentage] += 1
 
            
            # Identify hot commits
            #if percentage > self.hot_threshold_percentage.value:
                #eval_info['hot-commits'][commit] = percentage

                
        eval_info['avg-change-percentage'] = summed_avg_change_percentage / float(len(stats['commits']))
        eval_info['avg-changed-elements'] = summed_changed_elements / eval_info['nr-of-commits']
        eval_info['avg-total-elements'] = summed_total_elements / eval_info['nr-of-commits']
        



        eval_info['nr-hot-commits'] = len(eval_info['hot-commits'])

        with open(self.eval_data.path, "w+") as fd:
            fd.write(repr(eval_info))



        # Output the summary of this build into the statistics file.
        with open(self.stats.path, "w+") as fd:
            fd.write(repr(self.build_info))










        def plot_hash_count_histogram(hash_values, filename):
            dictionary = plt.figure()
            fig, ax = plt.subplots()
            plt.xlabel('Prozentanteil geaenderter Elemente')
            plt.ylabel('Anzahl von Commits')
            axes = plt.gca()
            axes.set_xlim([-10,100])
            axes.set_ylim([0,1600])

            ax.bar(hash_values.keys(), hash_values.values(), align='center')
            fig.savefig(filename)

        # clean data for plotting
        data = {k:v for k,v in eval_info['change-percentage'].iteritems() if k <= 100}
 
        plot_hash_count_histogram(data, self.hot_commits_histo.path)






    def variant_name(self):
        return "%s-%s"%(self.project_name(), self.metadata['mode'])

    def symlink_name(self):
        return "%s-%s"%(self.title, self.variant_name())


if __name__ == "__main__":
    experiment = HistoricalCompilationGlobalEvaluation()
    dirname = experiment(sys.argv)
