FROM gcc:latest AS builder
LABEL authors="aryan"
WORKDIR /app/
COPY . .
RUN gcc -o mock-shell src/*.c

FROM debian:bookworm-slim
WORKDIR /shell/
COPY --from=builder /app/mock-shell .
RUN apt-get update && apt-get install -y \
    coreutils \
    findutils \
    grep \
    iputils-ping \
    && rm -rf /var/lib/apt/lists/*
CMD ["./mock-shell"]