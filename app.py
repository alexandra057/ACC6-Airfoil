from flask import Flask, render_template, request, redirect, url_for, send_file
import mysql.connector as mysql
import hashlib
from tasks import compute_forces
from io import BytesIO
import sys
from time import sleep
import os

app = Flask(__name__, template_folder = "templates")

# Connect to db
db = mysql.connect(host = "db", user = "root", passwd = "12345",
                   database = "results", buffered = True, autocommit = True)
cursor = db.cursor()

cursor.execute("CREATE TABLE IF NOT EXISTS forces(hash TEXT, file BLOB);")

cursor.execute("SET GLOBAL wait_timeout=20000;")
cursor.execute("SET GLOBAL max_allowed_packet=128;")

@app.route('/')
def index():
    return(render_template("index.html"))


# Saves the received file from user to the database
@app.route('/upload', methods = ['POST', 'GET'])
def uppload_file():
    uploaded_file = request.files['file_to_upload'].read()
    # print(uploaded_file, file=sys.stderr)
    hash_value = hashlib.sha256(uploaded_file).hexdigest()

    # Check if the result is already in the database
    sql_query = f'SELECT * FROM forces WHERE hash = "{hash_value}";'
    cursor.execute(sql_query)
    row = cursor.fetchone()
    if (row is not None):
        file = BytesIO(row[1])
        return(send_file(file, attachment_filename = f'drag_lift_{hash_value}_saved.m', as_attachment = True))

    task = compute_forces.apply_async(args=[uploaded_file.decode("utf-8")])
    # Waiting for the task to complete
    while True:
        sleep(1)
        if task.ready():
            result = task.get()
            break

    # Save result in local directory
    file_name = f'drag_lift_{hash_value}.m'
    file_received = open(file_name, 'w+')
    file_received.write(result)
    file_received.write("\n")
    file_received.close()
    file_to_save = open(file_name, "r").read()

    # Store result in the database
    sql_query_new = "INSERT INTO forces(hash, file) VALUES(%s, %s)"
    blob_tuple = (hash_value, file_to_save)
    cursor.execute(sql_query_new, blob_tuple)

    return(send_file(file_name, attachment_filename=file_name, as_attachment = True))


if __name__ == "__main__":
    app.run(debug=True, host='0.0.0.0')
