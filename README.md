# degen_crypto

## Set up
Minimum gcc version >= 13 (C++20 above)
Python 3.10 above

run at root dir for this project:
```
./bootstrap.sh
```

## Arguments

### Run in debug mode
--debug            
-e <binance/bybit>

## ENVIRONMENT VARIABLES

1. To enable daily report email, 
```
export EMAIL_USER=<your email addr>
export EMAIL_PASSWORD=<your_app_password>
```
2. To enable trading various exchanges,
```
export BINANCE_API_KEY=<your binance api key>
export BINANCE_API_SECRET=<your binance api secret>
...
```