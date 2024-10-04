#!/bin/bash

docker build -t led_driver . || exit
CONTAINER_ID=`docker run -d -p 8000:8000 led_driver`
sleep 1s
rm -f main.ino.bin
wget http://0.0.0.0:8000/build/main.ino.bin
docker kill $CONTAINER_ID
docker rm $CONTAINER_ID
