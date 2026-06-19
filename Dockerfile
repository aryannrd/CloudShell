FROM gcc:latest AS builder
WORKDIR /app/
COPY . .
RUN apt-get update && apt-get install -y \
    libcurl4-openssl-dev \
    libcjson-dev
RUN gcc -Wall -Wextra -g -O0 -o mock-shell src/*.c -lcurl -lcjson

FROM python:3.12-slim
WORKDIR /app/
COPY --from=builder /app/mock-shell .
COPY api.py .
RUN apt-get update && apt-get install -y \
    libcjson1 \
    libcurl4 \
    && rm -rf /var/lib/apt/lists/*
RUN pip install fastapi uvicorn
ENV MOCK_SHELL_PATH=/app/mock-shell
CMD ["uvicorn", "api:app", "--host", "0.0.0.0", "--port", "8000"]