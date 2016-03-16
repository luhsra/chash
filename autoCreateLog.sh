#!/bin/bash

LOG_FILES="log_files"
START=0
COUNT=20
REFERENCE="reference/"



rm -rf workingcopy
git clone ${REFERENCE} workingcopy/
mkdir $LOG_FILES
cd workingcopy
git checkout HEAD~${START}

for ((z=$START;z<$(($START + $COUNT));z++))
do
	git checkout HEAD~1
	echo -e "\t\n" | make menuconfig
	make clean
	make -j 8
	mv clang-hash.log ../${LOG_FILES}/${z}.log
done
