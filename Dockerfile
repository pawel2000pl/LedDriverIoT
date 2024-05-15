FROM espressif/idf:release-v5.1

RUN mkdir /app
WORKDIR /app


COPY main /app/main
COPY CMakeLists.txt /app/CMakeLists.txt
COPY partitions.csv /app/partitions.csv
COPY resources /app/resources
COPY compile_resources.py /app/compile_resources.py

RUN python3 compile_resources.py

# RUN source /opt/esp/entrypoint.sh

CMD tail -f /dev/null
