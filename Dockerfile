FROM debian:12

RUN apt update
RUN apt install -y curl python3 python3-pip python3-serial
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

RUN arduino-cli config init
RUN arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
RUN arduino-cli core update-index
RUN arduino-cli board listall
RUN arduino-cli core install esp32:esp32

RUN arduino-cli --additional-urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

RUN mkdir -p /app
COPY resources /app/resources
COPY main /app/main
COPY compile_resources.py /app/compile_resources.py
WORKDIR /app
RUN python3 compile_resources.py
WORKDIR /app/main
RUN arduino-cli compile -b esp32:esp32:XIAO_ESP32C3 --build-property build.partitions=min_spiffs --build-property upload.maximum_size=1966080  ./main.ino

RUN mkdir -p /var/www/builds
RUN cp /app/main/build/esp32.esp32.XIAO_ESP32C3/* /var/www/builds/

WORKDIR /var/www/builds
RUN find -type f -not -name "*.md5" -exec bash -c "md5sum {} | head -c 32 > {}.md5" \;
RUN date > /var/www/builds/timestamp.txt

EXPOSE 8000
CMD python3 -m http.server --directory /var/www
