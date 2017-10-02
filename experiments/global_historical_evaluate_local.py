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

from dependency_analyzer import get_changed_functions_from_commit

'''
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
'''

class HistoricalCompilationEvaluation(Experiment):
    inputs = {
        "clang_hash": GitArchive("/home/cip/2015/yb90ifym/clang-hash/"),
        "project": GitArchive("/home/cip/2015/yb90ifym/clang-hash/hash-projects/lua"),
        "commits": Integer(4744),
        "jobs": Integer(1), # was 4
        "dataset": Directory("/home/cip/2015/yb90ifym/clang-hash/experiments/HistoricalCompilation-4e7c977077afea3d2ad77aeefe3b472c"), # full lua
        "hot_threshold_percentage": Integer(50), # minimal change percentage for commit to be classified as "hot"
    }
    outputs = {
        "stats": File("summary.dict"),
        "eval_data": File("eval.txt"),
        "hot_commits_histo": File("local_hot_commits.pdf"),
        "compare_approx_elem": File("local_compare_approx_elem.pdf"),
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

        
            total_changed_globals = 0 # How often was any global changed/introduced throughout the history?
            total_changed_records = 0 # How often was any record changed/introduced throughout the history?
            total_changed_static_funcs = 0 # How often was any static function changed/introduced throughout the history?
            total_changed_functions = 0 # How often was any function changed/introduced throughout the history? (incl. static)


            prev_commit = None
            prev_hashes = None
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

                local_hashes = {}
                for element in commit_data:
                    local_hashes[element[0]] = element[1]


                parent_hashes = {}
                if parent == prev_commit:
                    #print "Reuse prev_commit"
                    parent_hashes = prev_hashes
                else:
                    #print "Cannot reuse prev_commit"
                    parent_data = read_chash_data(parent)
                    for element in parent_data:
                        parent_hashes[element[0]] = element[1]
                                
                if not parent_hashes:
                    # If the data does not exist, note and skip
                    stats['data-empty'].add(commit)
                    
                    # Save data for reuse
                    prev_commit = commit
                    prev_hashes = local_hashes

                    continue

                #########################
                # LOCAL HASH EVALUATION #
                #########################
                
                commit_stats = {
                    'element-count' : len(local_hashes),
                    'changed-elements' : [],
                    'changed-functions-approx' : [],
                }

                # Get data from approximation
                cwd = os.getcwd()
                os.chdir(src_path)
                commit_stats['changed-functions-approx'] = get_changed_functions_from_commit(src_path, commit)
                os.chdir(cwd)

                elements = set(local_hashes.keys())
                parent_elements = set(parent_hashes.keys())
               
                commit_stats['changed-elements'] = set() #TODO here elements ^ parent_elements # elements either added or removed: if this is initialized with the insdel items, causes weird data to show um in result. should perhaps include it and add explanation
                
                # Compare hashes
                common_elements = elements & parent_elements
                for element in common_elements:
                    if local_hashes[element] != parent_hashes[element]:
                        commit_stats['changed-elements'].add(element)
                        if element.startswith('record:'): # do this here to ignore insertions and deletions
                            total_changed_records += 1
                        elif element.startswith('variable:') or element.startswith('static variable:'):
                            total_changed_globals += 1
                        elif element.startswith('static function:'):
                            total_changed_static_funcs += 1
                            total_changed_functions += 1
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


            self.build_info['stats'] = stats



        eval_info = {
            'nr-of-commits' : len(commits),
            'change-percentage' : {}, # change percentage -> nr of commits with change < percentage
            'hot-commits': {},
            'total-changed-globals': total_changed_globals,
            'total-changed-records': total_changed_records,
            'total-changed-static-funcs': total_changed_static_funcs,
            'total-changed-functions': total_changed_functions,
            'total-changed-elements': total_changed_functions + total_changed_records + total_changed_globals,
        }

        # Get most changed elements
        eval_info['most-changed-elements'] = {k:v for k,v in stats['elements'].iteritems() if v > self.commits.value/10} # arbitrary value (about 10% of commits)



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
            if percentage > self.hot_threshold_percentage.value:
                eval_info['hot-commits'][commit] = (percentage, len(commit_stat['changed-elements']), commit_stat['element-count'])

                
        eval_info['avg-change-percentage'] = summed_avg_change_percentage / float(len(stats['commits']))
        eval_info['avg-changed-elements'] = summed_changed_elements / eval_info['nr-of-commits']
        eval_info['avg-total-elements'] = summed_total_elements / eval_info['nr-of-commits']
        



        eval_info['nr-hot-commits'] = len(eval_info['hot-commits'])

        with open(self.eval_data.path, "w+") as fd:
            fd.write(repr(eval_info))



        # Output the summary of this build into the statistics file.
        with open(self.stats.path, "w+") as fd:
            fd.write(repr(self.build_info))


        '''
        def plot_hash_count_histogram(hash_values, filename):
            dictionary = plt.figure()
            fig, ax = plt.subplots()
            plt.xlabel('Prozentanteil geaenderter Elemente')
            plt.ylabel('Anzahl von Commits')
            ax.bar(hash_values.keys(), hash_values.values(), align='center')
            fig.savefig(filename)

        # clean data for plotting
        data = {k:v for k,v in eval_info['change-percentage'].iteritems() if k <= 100}
 
        plot_hash_count_histogram(data, self.hot_commits_histo.path)




        changed_funcs_approx_list = []
        changed_elements_list = []
        for commit in commits:
            commit_stat = commits[commit]
            changed_functions_approx = commit_stat['changed-functions-approx']
            changed_elements = commit_stat['changed-elements']
            
            changed_funcs_approx_list.append(len(changed_functions_approx))
            changed_elements_list.append(len(changed_elements))

        
        #TODO plot changed elements vs approx. changed functions
        # and also changed functions vs approx changed functions
        fig, ax = plt.subplots()
        ax.plot(changed_elements_list, label='Geaenderte Elemente (lokal)')
        ax.plot(changed_funcs_approx_list, 'm', label='Geaenderte Funktionen (Approx)')

        lgd = ax.legend(loc='center left', bbox_to_anchor=(1, 0.5)) # legend on the right
        plt.xlabel('Commits')
        plt.ylabel('Anzahl')
        fig.savefig(self.compare_approx_elem.path, bbox_extra_artists=(lgd,), bbox_inches='tight')
        '''



    def variant_name(self):
        return "%s-%s"%(self.project_name(), self.metadata['mode'])

    def symlink_name(self):
        return "%s-%s"%(self.title, self.variant_name())


if __name__ == "__main__":
    experiment = HistoricalCompilationEvaluation()
    dirname = experiment(sys.argv)
