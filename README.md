# ACC6-Airfoil

This application takes an .xml file as input and outputs an .m file containing the results of force calculations.


To run the application with one worker, run the following command:

```bash
$ docker-compose up
```
The number of workers is changeable. To run the application with n workers, enter this command:

```bash
$ docker-compose up --scale worker=n
```

To send a file to the application, go to localhost:5000 on your machine and upload a file.
When the application is done with the calculations, the result file will be downloaded to your machine.


