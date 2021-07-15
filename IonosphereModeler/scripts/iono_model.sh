#!/bin/bash

cmd="./ionosphere_modeler -r ../../testData/smooth -d ../../testData/dcb -c ../../testData/IONOREG.CRD -a ./almanac.yuma.week0711.589824.txt -s 1049847616 -e 1049848516 -i 900"
echo $cmd
$cmd