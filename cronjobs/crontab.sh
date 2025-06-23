# Use zsh as the shell
SHELL=/bin/zsh
TZ=Asia/Hong_Kong

# At 00:01 every day, just source the venv (optional)
1 0 * * * source ~/.zshrc && source ~/venv/bin/activate

# Send PnL report email at 6am, 12pm, 6pm, 12am
0 6,12,18,0 * * * TODAY=$(date +\%Y\%m\%d) && source ~/.zshrc && source ~/venv/bin/activate && python3 ~/degen_crypto/cronjobs/daily_report_email.py >> ~/degen_crypto/logs/$TODAY/email.log 2>&1

# Run degen_crypto setup
0 5 * * *  export TODAY=$(date +\%Y\%m\%d) && mkdir -p ~/degen_crypto/logs/$TODAY && source ~/.zshrc && cd ~/degen_crypto && ! pgrep -f "./build/release/degen_crypto" && ./bootstrap.sh && ./build/release/degen_crypto >> ~/degen_crypto/logs/$TODAY/run.log 2>&1