from flask import Flask, Response
from prometheus_client import Gauge, generate_latest, CONTENT_TYPE_LATEST, Info
from binance.client import Client
from datetime import datetime, timedelta
import os
import logging
from decimal import Decimal

app = Flask(__name__)

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

TOTAL_DEPOSIT_AUD = 7500
# Account info
account_info_gauge = Gauge("binance_pnl_info", "Total value in AUD", ["type"])

# Prometheus metrics
total_balance_gauge = Gauge("binance_total_balance_usdt", "Total account balance in USDT")
stable_coin_balance_gauge = Gauge("binance_stable_coin_usdt", "Total stable coin balance in USDT")
asset_balance_gauge = Gauge("binance_asset_balance", "Balance of individual assets", ["asset"])
total_pnl_gauge = Gauge("binance_total_pnl_usdt", "Total unrealized PnL in USDT")
trade_count_gauge = Gauge("binance_recent_trades_count", "Number of recent trades", ["symbol"])
account_info = Info("binance_account", "Binance account information")

# Futures metrics
futures_balance_gauge = Gauge("binance_futures_balance_usdt", "Futures account balance in USDT")
futures_pnl_gauge = Gauge("binance_futures_unrealized_pnl_usdt", "Futures unrealized PnL in USDT")
futures_position_gauge = Gauge("binance_futures_position_size", "Futures position size", ["symbol", "side"])
futures_position_pnl_gauge = Gauge("binance_futures_position_pnl", "Futures position PnL", ["symbol"])

# Trading metrics
daily_trades_gauge = Gauge("binance_daily_trades_count", "Number of trades in last 24h", ["symbol"])
trade_volume_gauge = Gauge("binance_trade_volume_usdt", "Trading volume in USDT (24h)", ["symbol"])

# Order book metrics
best_bid_gauge = Gauge("binance_best_bid_price", "Best bid price", ["symbol"])
best_ask_gauge = Gauge("binance_best_ask_price", "Best ask price", ["symbol"])
bid_ask_spread_gauge = Gauge("binance_bid_ask_spread", "Bid-ask spread", ["symbol"])

# Initialize Binance client
api_key = os.getenv("BINANCE_READONLY_API_KEY") 
api_secret = os.getenv("BINANCE_READONLY_API_SECRET")   

if not api_key or not api_secret:
    logger.error("Missing Binance API credentials. Set BINANCE_READONLY_API_KEY and BINANCE_READONLY_API_SECRET environment variables.")
    exit(1)

client = Client(api_key, api_secret)

def get_account_data():
    """Fetch account balance and asset information"""
    try:
        # Get account information
        account = client.get_account()

        # Calculate total balance in USDT
        total_balance_usdt = 0.0
        stable_coin_balance_usdt = 0.0
        asset_balances = {}
        
        # Get current prices for conversion to USDT
        tickers = {ticker['symbol']: float(ticker['price']) for ticker in client.get_all_tickers()}
        # work on currently holding assets
        for balance in account['balances']:
            asset = balance['asset']
            free = float(balance['free'])
            locked = float(balance['locked'])
            total = free + locked
            
            if total > 0:
                asset_balances[asset] = total
                
                # Convert to USDT value
                if asset == 'USDT':
                    usdt_value = total
                elif asset == 'BUSD':  # BUSD is roughly 1:1 with USDT
                    usdt_value = total
                else:
                    # Try to find ticker for asset/USDT pair
                    ticker_symbol = f"{asset}USDT"
                    if ticker_symbol in tickers:
                        usdt_value = total * tickers[ticker_symbol]
                    else:
                        # Try asset/BTC then BTC/USDT
                        btc_symbol = f"{asset}BTC"
                        if btc_symbol in tickers and 'BTCUSDT' in tickers:
                            btc_value = total * tickers[btc_symbol]
                            usdt_value = btc_value * tickers['BTCUSDT']
                        else:
                            usdt_value = 0  # Can't convert this asset
                
                total_balance_usdt += usdt_value
                if asset == 'USDT' or asset == 'FDUSD' or asset == 'USDC' or asset == 'USD1':
                    stable_coin_balance_usdt += usdt_value
        
        return {
            'total_balance_usdt': total_balance_usdt,
            'stable_coin_balance_usdt': stable_coin_balance_usdt,
            'asset_balances': asset_balances,
            'account_type': account.get('accountType', 'SPOT'),
            'can_trade': account.get('canTrade', False),
            'can_withdraw': account.get('canWithdraw', False),
            'can_deposit': account.get('canDeposit', False),
            'total_deposit_aud': TOTAL_DEPOSIT_AUD,
            'overall_pnl': round((total_balance_usdt * 1.55 - TOTAL_DEPOSIT_AUD) / TOTAL_DEPOSIT_AUD * 100, 2)
        }
    
    except Exception as e:
        logger.error(f"Error fetching account data: {e}")
        return None

def get_futures_data():
    """Fetch futures account data if available"""
    try:
        # Try to get futures account info
        futures_account = client.futures_account()
        
        total_wallet_balance = float(futures_account.get('totalWalletBalance', 0))
        total_unrealized_pnl = float(futures_account.get('totalUnrealizedProfit', 0))
        
        # Get position information
        positions = client.futures_position_information()
        position_data = {}
        
        for position in positions:
            symbol = position['symbol']
            position_amt = float(position['positionAmt'])
            unrealized_pnl = float(position['unRealizedProfit'])
            
            if position_amt != 0:  # Only track open positions
                side = 'LONG' if position_amt > 0 else 'SHORT'
                position_data[symbol] = {
                    'size': abs(position_amt),
                    'side': side,
                    'pnl': unrealized_pnl
                }
        
        return {
            'balance': total_wallet_balance,
            'unrealized_pnl': total_unrealized_pnl,
            'positions': position_data
        }
    
    except Exception as e:
        logger.warning(f"Could not fetch futures data (may not have futures enabled): {e}")
        return None

def get_orderbook_data():
    """Get best bid and ask prices for major trading pairs"""
    try:
        major_pairs = ['USDCUSDT', 'FDUSDUSDT', 'USD1USDT']
        orderbook_data = {}
        
        for symbol in major_pairs:
            try:
                # Get order book ticker (best bid/ask prices)
                ticker = client.get_orderbook_ticker(symbol=symbol)
                
                best_bid = float(ticker['bidPrice'])
                best_ask = float(ticker['askPrice'])
                
                # Calculate spread
                spread = best_ask - best_bid
                
                orderbook_data[symbol] = {
                    'best_bid': best_bid,
                    'best_ask': best_ask,
                    'spread': spread,
                }
                
            except Exception as e:
                logger.warning(f"Could not fetch orderbook for {symbol}: {e}")
                orderbook_data[symbol] = {
                    'best_bid': 0,
                    'best_ask': 0,
                    'spread': 0,
                }
        
        return orderbook_data
    
    except Exception as e:
        logger.error(f"Error fetching orderbook data: {e}")
        return {}

def get_recent_trades_summary():
    """Get summary of recent trades for major trading pairs"""
    try:
        # Get a few major trading pairs recent trades
        major_pairs = ['USDCUSDT', 'FDUSDUSDT', 'USD1USDT']
        trade_data = {}
        
        for symbol in major_pairs:
            try:
                # Get recent trades for this symbol (last 100 trades)
                trades = client.get_my_trades(symbol=symbol, limit=100)
                
                # Calculate 24h statistics
                import time
                current_time = int(time.time() * 1000)
                twenty_four_hours_ago = current_time - (24 * 60 * 60 * 1000)
                
                recent_trades = [t for t in trades if int(t['time']) >= twenty_four_hours_ago]
                
                # Calculate volume in USDT
                volume_usdt = sum(float(trade['quoteQty']) for trade in recent_trades)
                
                trade_data[symbol] = {
                    'count': len(recent_trades),
                    'volume_usdt': volume_usdt
                }
                
            except Exception as e:
                logger.warning(f"Could not fetch trades for {symbol}: {e}")
                trade_data[symbol] = {'count': 0, 'volume_usdt': 0}
        
        return trade_data
    
    except Exception as e:
        logger.error(f"Error fetching recent trades: {e}")
        return {}

@app.route("/metrics")
def metrics():
    try:
        # Get spot account data
        account_data = get_account_data()
        if account_data:
            # Update total balance
            total_balance_gauge.set(account_data['total_balance_usdt'])
            stable_coin_balance_gauge.set(account_data['stable_coin_balance_usdt'])
            account_info_gauge.labels(type='deposit').set(account_data['total_deposit_aud'])
            account_info_gauge.labels(type='pnl').set(account_data['overall_pnl'])

            # Update individual asset balances
            for asset, balance in account_data['asset_balances'].items():
                asset_balance_gauge.labels(asset=asset).set(balance)
            
            # Update account info
            account_info.info({
                'account_type': account_data['account_type'],
                'can_trade': str(account_data['can_trade']),
                'can_withdraw': str(account_data['can_withdraw']),
                'can_deposit': str(account_data['can_deposit'])
            })
        
        # # Get futures account data
        # futures_data = get_futures_data()
        # if futures_data:
        #     futures_balance_gauge.set(futures_data['balance'])
        #     futures_pnl_gauge.set(futures_data['unrealized_pnl'])
            
        #     # Update position metrics
        #     for symbol, pos_data in futures_data['positions'].items():
        #         futures_position_gauge.labels(symbol=symbol, side=pos_data['side']).set(pos_data['size'])
        #         futures_position_pnl_gauge.labels(symbol=symbol).set(pos_data['pnl'])
            
        #     # Set total PnL (spot + futures)
        #     total_pnl_gauge.set(futures_data['unrealized_pnl'])
        # else:
        #     # No futures data available
        #     futures_balance_gauge.set(0)
        #     futures_pnl_gauge.set(0)
        #     total_pnl_gauge.set(0)
        
        # Get recent trades summary
        trade_data = get_recent_trades_summary()
        for symbol, data in trade_data.items():
            daily_trades_gauge.labels(symbol=symbol).set(data['count'])
            trade_volume_gauge.labels(symbol=symbol).set(data['volume_usdt'])
        
        # Get orderbook data (best bid/ask)
        orderbook_data = get_orderbook_data()
        for symbol, data in orderbook_data.items():
            best_bid_gauge.labels(symbol=symbol).set(data['best_bid'])
            best_ask_gauge.labels(symbol=symbol).set(data['best_ask'])
            bid_ask_spread_gauge.labels(symbol=symbol).set(data['spread'])
        
        data = generate_latest()
        return Response(data, mimetype=CONTENT_TYPE_LATEST)
    
    except Exception as e:
        logger.error(f"Error generating metrics: {e}")
        # Return empty metrics on error
        data = generate_latest()
        return Response(data, mimetype=CONTENT_TYPE_LATEST)

@app.route("/")
def health_check():
    return "Binance Exporter is running"

@app.route("/health")
def health():
    """Health check endpoint"""
    try:
        # Test API connectivity
        client.get_server_time()
        return {"status": "healthy", "binance_api": "connected"}, 200
    except Exception as e:
        logger.error(f"Health check failed: {e}")
        return {"status": "unhealthy", "error": str(e)}, 500

def main():
    app.run(host="0.0.0.0", port=8000)

if __name__ == "__main__":
    main()
