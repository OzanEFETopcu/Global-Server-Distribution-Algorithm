#!/bin/bash

# Start the ec2 instance
aws ec2 start-instances --instance-ids "i-05e5afeea6b4f9877"

# Print the timestamp
date +%H:%M:%S:%N