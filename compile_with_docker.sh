#!/bin/bash
docker build --progress=plain -t led_driver . 2>&1 | tee logs.txt && ERROR=${PIPESTATUS[0]}
if [ $ERROR -ne 0 ]; then exit; fi
CONTAINER_ID=`docker run -d -p 8000:8000 led_driver`
sleep 1s
rm -f main.ino.bin
wget http://0.0.0.0:8000/build/main.ino.bin
docker kill $CONTAINER_ID
docker rm $CONTAINER_ID

CPP_CODE=`find main -type f \( -name "*.h" -or -name "*.cpp" \) -and -not -name "resources.*" -and -not -path "*/lib/*" -exec cat {} \; | wc -l`
JS_CODE=`find resources -type f -name "*.js" -and -not -path "*/lib/*" -exec cat {} \; | wc -l`
HTML_CODE=`find resources -type f -name "*.html" -and -not -path "*/lib/*" -exec cat {} \; | wc -l`
CSS_CODE=`find resources -type f -name "*.css" -and -not -path "*/lib/*" -exec cat {} \; | wc -l`
OTHER_CODE=`find resources -type f -name "*.svg" -and -not -path "*/lib/*" -or -name "*.json" -exec cat {} \; | wc -l`

MAIN_BYTES=`find main -type f -not -name "resources.*" -and -not -path "*/lib/*" -exec cat {} \; | wc -c`
RESOURCES_BYTES=`find resources -type f -exec cat {} \; | wc -c`
BIN_BYTES=`cat main.ino.bin | wc -c`

echo "Compilation successful"
echo "The code has (excluding libraries):"
echo "  $CPP_CODE lines in CPP"
echo "  $JS_CODE lines in JS"
echo "  $HTML_CODE lines in HTML"
echo "  $CSS_CODE lines in CSS"
echo "  $OTHER_CODE lines in other languages"
echo "Total "$(($CPP_CODE + $JS_CODE + $HTML_CODE + $CSS_CODE + $OTHER_CODE))" lines and "$(($MAIN_BYTES + $RESOURCES_BYTES))" bytes"
echo "The binary file has $BIN_BYTES bytes ($(($BIN_BYTES * 100 / 1966080))%)"
