#!/bin/bash -e

NUMKG=$1
EPG=$2
NUMCF=$3

./rocks_eval --numkg ${NUMKG} --epg ${EPG} --numcf ${NUMCF} -o write -c
./tools/ldb list_live_files_metadata --db=test
./rocks_eval --numkg ${NUMKG} --epg ${EPG} --numcf ${NUMCF} -o read
./rocks_eval --numkg ${NUMKG} --epg ${EPG} --numcf ${NUMCF} -o prefix_seek

./rocks_eval --numkg ${NUMKG} --epg ${EPG} --numcf ${NUMCF} -o delete
./rocks_eval --numkg ${NUMKG} --epg ${EPG} --numcf ${NUMCF} -o write
./tools/ldb list_live_files_metadata --db=test
./rocks_eval --numkg ${NUMKG} --epg ${EPG} --numcf ${NUMCF} -o read
./rocks_eval --numkg ${NUMKG} --epg ${EPG} --numcf ${NUMCF} -o prefix_seek

./rocks_eval --numkg ${NUMKG} --epg ${EPG} --numcf ${NUMCF} -o compact
./tools/ldb list_live_files_metadata --db=test
./rocks_eval --numkg ${NUMKG} --epg ${EPG} --numcf ${NUMCF} -o read
./rocks_eval --numkg ${NUMKG} --epg ${EPG} --numcf ${NUMCF} -o prefix_seek
