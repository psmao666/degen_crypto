# Dockerfile for binance-exporter
FROM python:3.10-slim

WORKDIR /app
COPY binance_exporter.py .

RUN pip install flask prometheus_client

CMD ["python", "binance_exporter.py"]

