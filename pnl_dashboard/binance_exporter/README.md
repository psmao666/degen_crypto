# Binance Prometheus Exporter

A Prometheus exporter for Binance account data including balances, trades, and PnL metrics.

## Features

- **Account Balance**: Total balance in USDT and individual asset balances
- **Futures Data**: Futures account balance, unrealized PnL, and position information
- **Trading Metrics**: Recent trade counts and volumes for major trading pairs
- **Position Tracking**: Open futures positions with size and PnL
- **Health Monitoring**: API connectivity and account status

## Setup

### 1. Install Dependencies

```bash
pip install -r requirements.txt
```

### 2. Set Environment Variables

Create a `.env` file or set environment variables:

```bash
export BINANCE_READONLY_API_KEY="your_api_key_here"
export BINANCE_READONLY_API_SECRET="your_api_secret_here"
```

**Important**: Use read-only API keys for security. The exporter only needs read permissions.

### 3. Test Connection

Before running the exporter, test your API connection:

```bash
python test_api.py
```

### 4. Run the Exporter

```bash
python binance_exporter.py
```

The exporter will start on `http://localhost:8000`

## API Endpoints

- `GET /metrics` - Prometheus metrics endpoint
- `GET /` - Basic health check
- `GET /health` - Detailed health check with API connectivity test

## Prometheus Metrics

### Account Metrics

| Metric | Type | Description | Labels |
|--------|------|-------------|---------|
| `binance_total_balance_usdt` | Gauge | Total account balance in USDT | - |
| `binance_asset_balance` | Gauge | Balance of individual assets | `asset` |
| `binance_account_info` | Info | Account information and permissions | - |

### Futures Metrics

| Metric | Type | Description | Labels |
|--------|------|-------------|---------|
| `binance_futures_balance_usdt` | Gauge | Futures account balance in USDT | - |
| `binance_futures_unrealized_pnl_usdt` | Gauge | Futures unrealized PnL in USDT | - |
| `binance_futures_position_size` | Gauge | Futures position size | `symbol`, `side` |
| `binance_futures_position_pnl` | Gauge | Futures position PnL | `symbol` |

### Trading Metrics

| Metric | Type | Description | Labels |
|--------|------|-------------|---------|
| `binance_daily_trades_count` | Gauge | Number of trades in last 24h | `symbol` |
| `binance_trade_volume_usdt` | Gauge | Trading volume in USDT (24h) | `symbol` |

### Overall Metrics

| Metric | Type | Description | Labels |
|--------|------|-------------|---------|
| `binance_total_pnl_usdt` | Gauge | Total unrealized PnL in USDT | - |

## Example Prometheus Configuration

Add this to your `prometheus.yml`:

```yaml
scrape_configs:
  - job_name: 'binance-exporter'
    static_configs:
      - targets: ['localhost:8000']
    scrape_interval: 30s
```

## Example Grafana Queries

### Total Portfolio Value
```promql
binance_total_balance_usdt + binance_futures_balance_usdt
```

### Top 5 Assets by Value
```promql
topk(5, binance_asset_balance)
```

### Futures PnL Percentage
```promql
(binance_futures_unrealized_pnl_usdt / binance_futures_balance_usdt) * 100
```

### Daily Trading Volume
```promql
sum by (symbol) (binance_trade_volume_usdt)
```

## Security Considerations

1. **Read-Only API Keys**: Only use API keys with read permissions
2. **IP Restrictions**: Consider restricting API keys to specific IP addresses
3. **Environment Variables**: Never commit API keys to version control
4. **Network Security**: Run the exporter in a secure network environment

## Troubleshooting

### Common Issues

1. **"Missing API credentials"**
   - Ensure environment variables are set correctly
   - Check that your `.env` file is loaded

2. **"Could not fetch futures data"**
   - This is normal if you don't have futures trading enabled
   - The exporter will still work for spot account data

3. **"Could not fetch trades for [SYMBOL]"**
   - This happens if you haven't traded that symbol recently
   - The metric will show 0, which is correct

### Debug Mode

Run with debug logging:

```bash
export PYTHONUNBUFFERED=1
python binance_exporter.py
```

## Docker Usage

You can also run this in Docker (see the Dockerfile in the parent directory):

```bash
docker-compose up binance-exporter
```

## Contributing

Feel free to submit issues and enhancement requests! 