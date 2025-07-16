FROM debian:bullseye

RUN apt-get update && apt-get install -y \
  gcc \
  make \
  libc-dev \
  git \
  curl \
  && apt-get clean

WORKDIR /app

COPY . .

RUN gcc server.c server_utils.c cjson/cJSON.c -o server.out -pthread

EXPOSE 8888

CMD ["./server.out"]

