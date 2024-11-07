#!/bin/bash

docker build -t led_driver . || exit
CONTAINER_ID=`docker run -d -p 8000:8000 led_driver`
sleep 1s
rm -f main.ino.bin
wget http://0.0.0.0:8000/build/main.ino.bin
docker kill $CONTAINER_ID
docker rm $CONTAINER_ID

CPP_CODE=`find main -type f \( -name "*.h" -or -name "*.cpp" \) -and -not -name "resources.*" -exec cat {} \; | wc -l`
JS_CODE=`find resources -type f -name "*.js" -exec cat {} \; | wc -l`
HTML_CODE=`find resources -type f -name "*.html" -exec cat {} \; | wc -l`
CSS_CODE=`find resources -type f -name "*.css" -exec cat {} \; | wc -l`
OTHER_CODE=`find resources -type f -name "*.svg" -or -name "*.json" -exec cat {} \; | wc -l`

MAIN_BYTES=`find main -type f -not -name "resources.*" -exec cat {} \; | wc -c`
RESOURCES_BYTES=`find resources -type f -exec cat {} \; | wc -c`
BIN_BYTES=`cat main.ino.bin | wc -c`

echo "Compilation successful"
echo "The code has:"
echo "  $CPP_CODE lines in CPP"
echo "  $JS_CODE lines in JS"
echo "  $HTML_CODE lines in HTML"
echo "  $CSS_CODE lines in CSS"
echo "  $OTHER_CODE lines in other languages"
echo "Total "$(($CPP_CODE + $JS_CODE + $HTML_CODE + $CSS_CODE + $OTHER_CODE))" lines and "$(($MAIN_BYTES + $RESOURCES_BYTES))" bytes"
echo "The binary file has $BIN_BYTES bytes ($(($BIN_BYTES * 100 / 1966080))%)"
