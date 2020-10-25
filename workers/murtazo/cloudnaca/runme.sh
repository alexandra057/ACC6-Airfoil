#!/bin/bash
# runme.sh
# DESCRIPTION
# Script to call naca2gmsh_geo.py repetitetively and generate meshes
# for GMSH with differenr angles of attack on a specified NACA four
# digit airfoil
#
# ARGUMENTS
# angle_start : smallest anglemof attack (degrees)
# angle_stop  : biggest angle of attack (degrees)
# n_angles    : split angle_stop-angle_start into n_angles parts
# n_nodes     : number of nodes on one side of airfoil
# n_levels    : number of refinement steps in meshing 0=no refinement 1=one time 2=two times etc...
#
# EXAMPLE
# ./runme.sh 0 30 10 200 3
#
# EDIT FOLLOWING VARIABLES
# Path to GMSH binary
GMSHBIN="/usr/bin/gmsh"
# Path to dir where geo files will be stored
GEODIR="geo"
# Path to dir where msh files will be stored
MSHDIR="msh"
# NACA four digit airfoil (typically NACA0012)
NACA1=0
NACA2=0
NACA3=1
NACA4=2
###########################################################################
angle_start="$1"
angle_stop="$2"
n_angles="$3"
n_nodes="$4"
n_levels="$5"

anglediff=$((($angle_stop-$angle_start)/$n_angles))
for i in `seq 0 $n_angles`;
do
  angle=$(($angle_start + $anglediff*i))
  geofile=a${angle}n${n_nodes}.geo
  ./naca2gmsh_geo.py $NACA1 $NACA2 $NACA3 $NACA4 $angle $n_nodes > $GEODIR/$geofile
done
for i in `ls $GEODIR`; do
  mshfile="$(echo $i|sed -e 's/geo/msh/')";
  $GMSHBIN -v 0 -nopopup -2 -o $MSHDIR/r0$mshfile $GEODIR/$i;
done

if [ "$n_levels" -gt "0" ]; then
  for i in `seq 1 $n_levels`; do
    pm=r$(($i-1))a;
    pmn=r$(($i))a;
    for j in `ls $MSHDIR|grep ^$pm`; do
      newname="$(echo $j|sed -e s/"$pm"/"$pmn"/)"; 
      cp $MSHDIR/$j $MSHDIR/$newname;
      $GMSHBIN -refine -v 0 $MSHDIR/$newname;
    done
  done
fi
