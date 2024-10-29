#!/bin/bash

# Launching an ec2 instance
aws ec2 run-instances --image-id ami-0fd660f8ce5d4ba85 --count 1 --instance-type c6g.xlarge --key-name EC2BootTestKeyPair --security-group-ids sg-03f79d75a1a1a388f
