Files:
readme.txt: this file, instructions
runme.sh: bash script controlling the execution of generating GMSH .msh mesh files
naca2gmsh_geo.py: python script generatimng GMSH .geo geometry files
geo: directory where geo files are stored
msh: directory where msh files are stored

TODO:
1) install GMSH: http://geuz.org/gmsh/
2) edit paths and variables in runme.sh
3) edit path to python in shebang of naca2gmsh_geo.py
4) run runme.sh with appropriate arguments (se runme.sh for example)


Note:
If change of density of mesh is wanted, edit variables lc1 (airfoil) and lc2 (outer boundary) in naca2gmsh_geo.py
