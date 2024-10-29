#!/bin/bash

# Start the compute instance
gcloud compute instances start instance-20241029-182024 --zone europe-west2-b

# Print the timestamp
date +%H:%M:%S:%N