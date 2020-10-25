from celery import Celery
import os
import time
import shutil

client = Celery("tasks", backend = "rpc://", broker="pyamqp://u:p@rabbit:5672//")

@client.task
def compute_forces(uploaded_file):
    # Save file to local directory
    os.chdir("navier_stokes_solver")
    file_name = "mesh.xml"
    file_to_save = open("mesh.xml", "w+")
    file_to_save.write(uploaded_file)
    file_to_save.close()
    os.system("./airfoil 10 0.0001 10. 1 mesh.xml")

    os.chdir("results")
    file_name_return = "drag_ligt.m"
    file_to_return = open("drag_ligt.m", "r")
    string_to_return = file_to_return.read()

    # Remove saved files from container when we are done with the computations
    os.chdir("..")
    shutil.rmtree("results")
    os.remove("mesh.xml")
    os.chdir("..")
    return(string_to_return)
