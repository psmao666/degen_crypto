import smtplib
from email.message import EmailMessage
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import encoders
import os
from datetime import datetime

EMAIL_ADDRESS = os.environ['EMAIL_USER']       # Your Gmail address
EMAIL_PASSWORD = os.environ['EMAIL_PASSWORD']  # App password

# Get today's date in YYYYMMDD format
today = datetime.now().strftime('%Y%m%d')
log_filename = f'controller_{today}.log'
log_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), "logs", today, log_filename)

msg = MIMEMultipart()
msg['Subject'] = '[Degen Crypto] Daily P&L Report'
msg['From'] = EMAIL_ADDRESS
msg['To'] = 'pengsen.mao88@gmail.com'

# Email body
body = 'Hello Degen, this is your daily P&L report from Degen Crypto.'
msg.attach(MIMEText(body, 'plain'))

# Helper function to attach a file
def attach_file(msg, filepath, filename):
    if os.path.exists(filepath):
        with open(filepath, 'rb') as attachment:
            part = MIMEBase('application', 'octet-stream')
            part.set_payload(attachment.read())
        encoders.encode_base64(part)
        part.add_header(
            'Content-Disposition',
            f'attachment; filename= {filename}'
        )
        msg.attach(part)
        print(f"Attached file: {filename}")
    else:
        print(f"File not found: {filepath}")

# Attach controller log file
attach_file(msg, log_path, log_filename)

# Attach trades.log file from the same directory as this script
trades_log_path =  os.path.join(os.path.dirname(os.path.dirname(__file__)), "logs", today, "trades.log")
attach_file(msg, trades_log_path, 'trades.log')

# Attach errors.log file from the same directory as this script
errors_log_path =  os.path.join(os.path.dirname(os.path.dirname(__file__)), "logs", today, "errors.log")
attach_file(msg, errors_log_path, 'errors.log')

# Send the email
try:
    with smtplib.SMTP_SSL('smtp.gmail.com', 465) as smtp:
        smtp.login(EMAIL_ADDRESS, EMAIL_PASSWORD)
        smtp.send_message(msg)
    print("Email sent successfully.")
except Exception as e:
    print("Failed to send email:", e)
