SHELL=/bin/zsh
CRON_TZ=Australia/Sydney

# Send PnL report email at 6am, 12pm, 6pm, 12am
# grep errors and trades
58 6,12,18,23 * * * grep "ERROR" /tmp/console_*_$(date +\%Y\%m\%d).log > ~/degen_crypto/logs/$(date +\%Y\%m\%d)/errors.log
58 6,12,18,23 * * * grep "Order filled" ~/degen_crypto/logs/$(date +\%Y\%m\%d)/controller_$(date +\%Y\%m\%d).log > ~/degen_crypto/logs/$(date +\%Y\%m\%d)/trades.log

59 6,12,18,23 * * * source ~/.zshrc && source ~/venv/bin/activate && python3 ~/degen_crypto/cronjobs/daily_report_email.py >> ~/degen_crypto/logs/$(date +\%Y\%m\%d)/email.log 2>&1

# Run degen_crypto binance instance
2 0 * * * source ~/.zshrc && cd ~/degen_crypto && ./build/binance/release/degen_crypto >> /tmp/console_binance_$(date +\%Y\%m\%d).log 2>&1

# Kill degen_crypto instance
58 23 * * * pkill -f ./build/binance/release/degen_crypto

# Archive logs
59 23 * * * cd ~/degen_crypto/logs/$(date +\%Y\%m\%d) && gzip *.log