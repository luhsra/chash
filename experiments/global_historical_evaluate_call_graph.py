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

from dependency_analyzer import get_changed_functions_from_commit
from dependency_analyzer import get_impacted_funcs_fake_hash


import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt


class HistoricalCompilationCallGraphEvaluation(Experiment):
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
        "hot_commits_histo": File("cg_hot_commits.pdf"),
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




            total_changed_functions = 0 # How often was any function changed throughout the history?

            total_insdel_functions = 0 # How often was any function introduced/removed throughout the history?


            prev_commit = None
            prev_functions = None
            prev_used_definitions = None
            counter = 1
            for info in commits:
                print "%d/%d" % (counter, len(commits))
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

                functions = set()
                used_definitions = {}
                for element in commit_data:
                    if element[0].startswith('static function:') or element[0].startswith('function:'):
                        clean_name = element[0].split(':')[1]
                        functions.add(clean_name)
                        used_definitions[clean_name] = set()
                        for used_def in element[2]:
                            if used_def.startswith('static function:') or used_def.startswith('function:'):
                                used_definitions[clean_name].add(used_def.split(':')[1])


                parent_functions = {}
                parent_used_definitions = {}
                if parent == prev_commit and prev_functions and prev_used_definitions:
                    #print "Reuse prev_commit"
                    parent_functions = prev_functions
                    parent_used_definitions = prev_used_definitions
                else:
                    #print "Cannot reuse prev_commit"
                    parent_data = read_chash_data(parent)
                    for element in parent_data:
                        if element[0].startswith('static function:') or element[0].startswith('function:'):
                            clean_name = element[0].split(':')[1]
                            parent_functions.insert(clean_name)
                            parent_used_definitions[clean_name] = set()
                            for used_def in element[2]:
                                if used_def.startswith('static function:') or used_def.startswith('function:'):
                                    parent_used_definitions[clean_name].add(used_def.split(':')[1])

                                
                if not parent_functions:
                    # If the data does not exist, note and skip
                    stats['data-empty'].add(commit)
                    
                    # Save data for reuse
                    prev_commit = commit
                    prev_functions = functions
                    prev_used_definitions = used_definitions
                    continue

                #########################
                # CALL GRAPH EVALUATION #
                #########################
                
                commit_stats = {
                    'element-count' : len(functions),
                    'changed-elements' : [], # contains changed + impacted functions
                    #'changed-not-impacted': set(), # contains directly changed functions only
                }


                elements = functions
                parent_elements = parent_functions
               
                commit_stats['changed-elements'] = set() #elements ^ parent_elements # elements either added or removed

                total_insdel_functions += len(commit_stats['changed-elements'])

                cwd = os.getcwd()
                os.chdir(src_path)
                changed_functions = get_changed_functions_from_commit(src_path, commit)
                os.chdir(cwd)

                commit_stats['changed-not-impacted'] = changed_functions.copy()
                
                # Get impacted functions
                changed_functions |= get_impacted_funcs_fake_hash(changed_functions, used_definitions) 

                commit_stats['changed-elements'] |= changed_functions

                total_changed_functions += len(changed_functions)

                commit_stats['changed-element-count'] = len(commit_stats['changed-elements']);
                stats['commits'][commit] = commit_stats


                # Count how often each element was changed over the whole history
                for element in commit_stats['changed-elements']:
                    if element not in stats['elements']:
                        stats['elements'][element] = 0;
                    stats['elements'][element] += 1


                # Save data for reuse
                prev_commit = commit
                prev_functions = functions
                prev_used_definitions = used_definitions

            self.build_info['stats'] = stats



        eval_info = {
            'nr-of-commits' : len(commits),
            'change-percentage' : {}, # change percentage -> nr of commits with change < percentage
            'hot-commits': {},
            'total-changed-functions': total_changed_functions,
            'total-insdel-functions': total_insdel_functions,
        }

        # Get most changed elements
        eval_info['most-changed-elements'] = {k:v for k,v in stats['elements'].iteritems() if v > 400} # arbitrary value (about 10% of commits)
        
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
    experiment = HistoricalCompilationCallGraphEvaluation()
    dirname = experiment(sys.argv)
