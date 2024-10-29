#!/bin/bash

# Ensure that flask is installed
/usr/bin/pip3 install flask

# Running the python application
nohup python3 -m flask --app /home/ec2-user/Global-Server-Distribution-Algorithm/serverMeasurements/measurementServerSide.py \
run --host 0.0.0.0 --port 8000 > /home/ec2-user/flask_app.log 2>&1

