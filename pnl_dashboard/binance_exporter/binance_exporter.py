from flask import Flask, Response
from prometheus_client import Gauge, generate_latest, CONTENT_TYPE_LATEST
import random

app = Flask(__name__)

pnl_gauge = Gauge("binance_pnl", "Total PnL from Binance account")

@app.route("/metrics")
def metrics():
    pnl = random.uniform(-100, 100)  # Replace with real computation
    pnl_gauge.set(pnl)
    data = generate_latest()
    return Response(data, mimetype=CONTENT_TYPE_LATEST)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8000)

