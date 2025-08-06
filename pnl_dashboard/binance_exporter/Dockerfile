# Dockerfile for binance-exporter
FROM python:3.10-slim

WORKDIR /app
COPY requirements.txt .
COPY binance_exporter.py .

RUN pip install -r requirements.txt 

CMD ["python", "binance_exporter.py"]

