FROM debian:12

RUN apt update
RUN apt install -y curl python3 python3-pip python3-serial
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

RUN arduino-cli config init
RUN arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
RUN arduino-cli core update-index
RUN arduino-cli board listall
RUN arduino-cli core install esp32:esp32
RUN arduino-cli lib install ArduinoJson

RUN mkdir -p /app
COPY doc /app/doc
COPY resources /app/resources
COPY main /app/main
COPY License.txt /app/License.txt
COPY compile_resources.py /app/compile_resources.py
WORKDIR /app
RUN python3 compile_resources.py
WORKDIR /app/main
RUN arduino-cli compile -b esp32:esp32:XIAO_ESP32C3 --build-property build.partitions=min_spiffs --build-property upload.maximum_size=1966080 ./main.ino

RUN mkdir -p /var/www/build
RUN cp /tmp/arduino/sketches/*/main* /var/www/build/
RUN cp -r /app/doc /var/www/doc
RUN cp /app/License.txt /var/www/doc/license.txt
RUN cp /app/License.txt /var/www/build/license.txt
RUN cp /app/doc/index.html /var/www/index.html
RUN cp /app/resources/favicon.svg /var/www/favicon.svg

WORKDIR /var/www/build
RUN find -type f -not -name "*.md5" -and -not -name "*.size" -exec bash -c "md5sum {} | head -c 32 > {}.md5" \;
RUN find -type f -not -name "*.md5" -and -not -name "*.size" -exec bash -c "du {} | grep -oP '^[0-9]*' > {}.size" \;
RUN date > /var/www/build/timestamp.txt
RUN cp -r /app/resources/version.json /var/www/build/version.json

EXPOSE 8000
CMD python3 -m http.server --directory /var/www
