FROM ubuntu
RUN apt-get update
RUN apt-get -y upgrade 
RUN apt-get install -y python3-pip
RUN pip3 install --upgrade pip
RUN pip3 install mysql-connector-python flask
RUN pip3 install celery
CMD ["python3","app.py"]
RUN mkdir /home/app
RUN mkdir /home/app/templates
WORKDIR /home/app
COPY templates /home/app/templates
COPY tasks.py /home/app
COPY app.py /home/app
