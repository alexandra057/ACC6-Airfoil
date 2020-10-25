FROM quay.io/fenicsproject/stable:current
RUN apt-get update
RUN apt-get install python3-pip -y
RUN pip3 install --upgrade pip
RUN pip3 install celery
RUN apt-get install python-numpy
RUN apt-get install gmsh -y
COPY murtazo .
COPY steps.sh .
RUN chmod +x steps.sh
RUN ./steps.sh
COPY tasks.py .
ENTRYPOINT celery -A tasks worker --loglevel=INFO -O fair --concurrency=20
