FROM debian:12

RUN apt update
RUN apt install -y curl git python3 python3-pip python3-serial
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

RUN arduino-cli config init
RUN arduino-cli config set network.connection_timeout 0
RUN arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
RUN arduino-cli core update-index
RUN arduino-cli board listall
RUN arduino-cli core install esp32:esp32
RUN arduino-cli lib install ArduinoJson
RUN git clone https://github.com/meshtastic/esp32_https_server /root/Arduino/libraries/esp32_https_server

RUN mkdir -p /app
COPY compilation_utils /app/compilation_utils
COPY doc /app/doc
COPY resources /app/resources
COPY main /app/main
COPY License.txt /app/License.txt
WORKDIR /app
RUN python3 compilation_utils/validate_json.py /app/resources/default_config.json /app/resources/config.schema.json
RUN python3 compilation_utils/compile_resources.py
RUN g++ compilation_utils/validate_config.cpp main/src/validate_json.cpp `find /root/Arduino/libraries/ArduinoJson -type d -exec echo -I{} -L{} \;` -o validate_configuration
RUN ./validate_configuration
RUN mkdir -p /tmp/app-build
WORKDIR /app/main
RUN arduino-cli compile -b esp32:esp32:esp32c3:CDCOnBoot=cdc,PartitionScheme=min_spiffs --warnings all --build-property compiler.optimization_flags=-Os --build-property upload.maximum_size=1966080 --build-property compiler.cpp.extra_flags="-DHTTPS_LOGLEVEL=0 -MMD -c" --output-dir /tmp/app-build ./main.ino 2>&1

RUN mkdir -p /var/www/build
RUN cp /tmp/app-build/main* /var/www/build/
RUN cp -r /app/doc /var/www/doc
RUN cp /app/License.txt /var/www/doc/license.txt
RUN cp /app/License.txt /var/www/build/license.txt
RUN cp /app/compilation_utils/index.html /var/www/index.html
RUN cp /app/resources/favicon.svg /var/www/favicon.svg

WORKDIR /var/www/build
RUN find -type f -not -name "*.md5" -and -not -name "*.size" -exec bash -c "md5sum {} | head -c 32 > {}.md5" \;
RUN find -type f -not -name "*.md5" -and -not -name "*.size" -exec bash -c "du {} | grep -oP '^[0-9]*' > {}.size" \;
RUN date > /var/www/build/timestamp.txt
RUN cp -r /app/resources/version.json /var/www/build/version.json

EXPOSE 8000
CMD python3 -m http.server --directory /var/www
