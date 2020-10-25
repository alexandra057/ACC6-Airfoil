#!/usr/bin/python
###########################################################################
#
# Name: naca2gmsh_geo.py
# Argumenmts (6):
#   naca1: first digit NACA four digit
#   naca2: second digit NACA four digit
#   naca3: third digit NACA four digit
#   naca4: fourth digit NACA four digit
#   angle: angle of attack
#   n_nodes: number of nodes in x-wise direction
# Output: to stdout
# Example: ./naca2gmsh_geo.py 0 0 1 2 10 100 > mygeo.geo
#
###########################################################################
import re, sys, numpy as np

###########################################################################
# Rotating airfoil
# Arguments coordinates x,y and angle a in degrees
###########################################################################
def rot(x,y,a):
  ar = -a*3.14159/180
  xa = x*np.cos(ar)-y*np.sin(ar)
  ya = y*np.cos(ar)+x*np.sin(ar)
  return xa, ya

###########################################################################
# Generate coordinates for NACA four digit airfoil
# n0,n1,n2,n3 are the four digits (standard airfoirl 0,0,1,2
# x is the desired x-coordinates array
###########################################################################
def naca4(n0,n1,n2,n3,x):
  m = n0 / 100.0
  p = n1 / 10.0
  t = (10 * n2 + n3) / 100.0
  c = 1.0
# Closed trailing edge, change -0.1036 to -0.1015 for original def
  yt = 5*t*c*(0.2969*np.sqrt(x/c)+(-0.1260)*x/c+(-0.3516)*pow(x/c,2)+0.2843*pow(x/c,3)+(-0.1036)*pow(x/c,4))
  yc = x.copy()
  i = 0
  for xx in x:
    if xx < p*c:
      yc[i]=m*xx/p/p*(2*p-xx/c)
    else:
      yc[i]=m*(c-xx)/pow(1-p,2)*(1+xx/c-2*p)
    i += 1
  upper = yt+yc
  lower = -yt+yc
  xreturn = np.append(x,x[x.size-2:0:-1])
  yreturn = np.append(upper,lower[lower.size-2:0:-1])
  return xreturn,yreturn

###########################################################################
# Generate GMSH geo format from coordinates
###########################################################################
def dat2gmsh(x,y):
  lc1 = 0.01
  lc2 = 1.00
  i = 0
  while i < x.size:
    print "Point(" + str(i+1) + ") = {" + str(x[i]) + "," + str(y[i])+",0,"+str(lc1)+"};"
    i += 1 
  ntot = x.size
  i = 1
  while i < ntot:
    print "Line(" + str(i) + ")={" + str(i) + "," + str(i+1) +"};"
    i += 1
  print "Line(" + str(ntot) + ")={" + str(ntot) + "," + "1};"
  print "Line Loop(%s)={" % (ntot+1),
  i = 1
  while i < ntot:
    print("%d," % (i)),
    i += 1 
  print str(ntot) + "};"
# Outer domain boundary
  print "Point(100000) = {-10,0,0,"+str(lc2)+"};"
  print "Point(101000) = {0,10,0,"+str(lc2)+"};"
  print "Point(102000) = {10,0,0,"+str(lc2)+"};"
  print "Point(103000) = {0,-10,0,"+str(lc2)+"};"
  print "Point(104000) = {0,0,0,"+str(lc2)+"};"
  print "Circle(105000) = {100000,104000,101000};"
  print "Circle(106000) = {101000,104000,102000};"
  print "Circle(107000) = {102000,104000,103000};"
  print "Circle(108000) = {103000,104000,100000};"
  print "Line Loop(109000) = {105000,106000,107000,108000};"
  print "Plane Surface(110000) = {109000,"+str(ntot+1)+"};"

###########################################################################
# Main loop
###########################################################################
if len(sys.argv) != 7:
  sys.exit("Usage: naca2gmsh_geo.py angle n_nodes")
n1 = float(sys.argv[1])
n2 = float(sys.argv[2])
n3 = float(sys.argv[3])
n4 = float(sys.argv[4])
angle = float(sys.argv[5])
n_nodes = int(sys.argv[6])

xs = np.linspace(0.0,1.0,n_nodes)
x, y = naca4(n1,n2,n3,n4,xs)
xa, ya = rot(x,y,angle)
dat2gmsh(xa,ya)
###########################################################################

