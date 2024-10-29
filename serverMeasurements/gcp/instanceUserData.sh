#!/bin/bash

# Ensure that flask is installed
sudo apt install python3-flask

# Running the python application
nohup python3 -m flask --app /home/ozanefe_topcu/Global-Server-Distribution-Algorithm/serverMeasurements/measurementServerSide.py \
run --host 0.0.0.0 --port 8000 > /home/ozanefe_topcu/flask_app.log 2>&1

