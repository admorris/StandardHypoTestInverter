#!/bin/bash
#$ -N "BdPhiPhi_CLs"
#$ -l h_rt=24:00:00
#$ -l h_vmem=4G
#$ -pe smp 4
#$ -cwd
source /cvmfs/lhcb.cern.ch/group_login.sh -c x86_64-centos7-gcc49-opt
. SetupProject.sh ROOT 6.06.02
bin/main /Disk/ds-sopa-group/PPE/lhcb/users/emmy/CLs/savedWorkspace.root w SB_model data --PoiMin 1e-9 --PoiMax 1e-8

