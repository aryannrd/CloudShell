FROM gcc:latest AS builder
LABEL authors="aryan"
WORKDIR /app/
COPY . .
RUN apt-get update && apt-get install -y \
    libcurl4-openssl-dev \
    libcjson-dev
RUN gcc -Wall -Wextra -g -O0 -o mock-shell src/*.c -lcurl -lcjson

FROM debian:bookworm-slim
WORKDIR /shell/
COPY --from=builder /app/mock-shell .
ENV HOME=/root
RUN apt-get update && apt-get install -y \
    libcjson1 \
    libcurl4 \
    coreutils \
    findutils \
    grep \
    iputils-ping \
    curl \
    && rm -rf /var/lib/apt/lists/*
CMD ["./mock-shell"]