# Use zsh as the shell
SHELL=/bin/zsh
TZ=Asia/Hong_Kong
TODAY=$(date +\%Y\%m\%d)

# At 00:01 every day, just source the venv (not necessary, shown here for completeness)
1 0 * * * source ~/.zshrc && source ~/venv/bin/activate

# Send pnl report email at 6:00am, 12:00pm, 6:00pm, 12:00am
0 6,12,18,0 * * * source ~/.zshrc && source ~/venv/bin/activate && python ~/degen_crypto/cronjobs/daily_report_email.py >> ~/degen_crypto/logs/$TODAY/email.log 2>&1

# Run degen_crypto setup and binary at 5:00am
0 5 * * * source ~/.zshrc && cd ~/degen_crypto && ./bootstrap.sh && ./build/release/degen_crypto        