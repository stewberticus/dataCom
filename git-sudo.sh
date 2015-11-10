#!/bin/sh

echo "Pulling"
git pull

echo "Sudoing"
sudo python prj4-net.py
