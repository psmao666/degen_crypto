#!/usr/bin/env python3
"""
Test script to verify Binance API connectivity and data retrieval
"""

import os
import sys
from binance.client import Client

def test_binance_connection():
    """Test basic Binance API connectivity"""
    
    # Get API credentials
    api_key = os.getenv("BINANCE_READONLY_API_KEY")
    api_secret = os.getenv("BINANCE_READONLY_API_SECRET")
    
    if not api_key or not api_secret:
        print("❌ Error: Missing API credentials")
        print("Please set BINANCE_READONLY_API_KEY and BINANCE_READONLY_API_SECRET environment variables")
        return False
    
    try:
        # Initialize client
        client = Client(api_key, api_secret)
        print("✅ Binance client initialized")
        
        # Test server connectivity
        server_time = client.get_server_time()
        print(f"✅ Server connection: {server_time}")
        
        # Test account access
        account = client.get_account()
        print(f"✅ Account access: Type={account.get('accountType', 'SPOT')}")
        
        # Show account balances
        print("\n📊 Account Balances:")
        balances = [b for b in account['balances'] if float(b['free']) > 0 or float(b['locked']) > 0]
        
        if balances:
            for balance in balances[:10]:  # Show first 10 non-zero balances
                free = float(balance['free'])
                locked = float(balance['locked'])
                total = free + locked
                print(f"  {balance['asset']}: {total:.8f} (free: {free:.8f}, locked: {locked:.8f})")
        else:
            print("  No balances found")
        
        # Test futures access (optional)
        print("\n🔮 Futures Account Test:")
        try:
            futures_account = client.futures_account()
            balance = float(futures_account.get('totalWalletBalance', 0))
            pnl = float(futures_account.get('totalUnrealizedProfit', 0))
            print(f"  ✅ Futures Balance: {balance:.2f} USDT")
            print(f"  📈 Unrealized PnL: {pnl:.2f} USDT")
        except Exception as e:
            print(f"  ⚠️  Futures not accessible: {e}")
        
        # Test recent trades for a major pair
        print("\n📈 Recent Trades Test:")
        try:
            trades = client.get_my_trades(symbol='BTCUSDT', limit=5)
            print(f"  ✅ Retrieved {len(trades)} recent BTCUSDT trades")
        except Exception as e:
            print(f"  ⚠️  Could not fetch trades: {e}")
        
        return True
        
    except Exception as e:
        print(f"❌ Connection failed: {e}")
        return False

def main():
    print("🚀 Testing Binance API Connection\n")
    
    success = test_binance_connection()
    
    if success:
        print("\n✅ All tests passed! Your Binance exporter should work correctly.")
    else:
        print("\n❌ Tests failed. Please check your API credentials and permissions.")
        sys.exit(1)

if __name__ == "__main__":
    main() 