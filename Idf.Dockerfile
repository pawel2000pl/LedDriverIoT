FROM espressif/idf:release-v5.1

RUN mkdir /app
WORKDIR /app

RUN mkdir -p /app
COPY CMakeLists.txt /app/CMakeLists.txt
COPY partitions.csv /app/partitions.csv
COPY sdkconfig /app/sdkconfig
COPY compilation_utils /app/compilation_utils
COPY doc /app/doc
COPY resources /app/resources
COPY main /app/main
COPY License.txt /app/License.txt
WORKDIR /app
RUN python3 compilation_utils/validate_json.py /app/resources/default_config.json /app/resources/config.schema.json
RUN python3 compilation_utils/compile_resources.py
RUN python3 compilation_utils/show_version.py > version.txt
RUN g++ compilation_utils/validate_config.cpp main/src/validate_json.cpp -o validate_configuration
RUN ./validate_configuration

RUN bash -c 'source /opt/esp/idf/export.sh && idf.py build'

RUN mkdir -p /var/www/build
RUN cp /app/build/iot-led-driver* /var/www/build/
RUN cp -r /app/doc /var/www/doc
RUN cp /app/License.txt /var/www/doc/license.txt
RUN cp /app/License.txt /var/www/build/license.txt
RUN cp /app/compilation_utils/index.html /var/www/index.html
RUN cp /app/resources/favicon.svg /var/www/favicon.svg

WORKDIR /var/www/build
RUN find -type f -not -name "*.md5" -and -not -name "*.size" -exec bash -c "md5sum {} | head -c 32 > {}.md5" \;
RUN find -type f -not -name "*.md5" -and -not -name "*.size" -exec bash -c "du {} | grep -oP '^[0-9]*' > {}.size" \;
RUN for f in main.ino.*; do mv -- "$f" "iot-led-driver.${f#main.ino.}"; done || echo skip
RUN date > /var/www/build/timestamp.txt
RUN cp -r /app/resources/version.json /var/www/build/version.json

EXPOSE 8000
CMD ["python3", "-m", "http.server", "--directory", "/var/www"]
