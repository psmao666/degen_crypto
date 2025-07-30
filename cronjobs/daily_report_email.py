import smtplib
from email.message import EmailMessage
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import encoders
import os
from datetime import datetime

EMAIL_ADDRESS = os.environ['EMAIL_USER']       # Your Gmail address
EMAIL_PASSWORD = os.environ['EMAIL_PASSWORD']      # App password

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

# Attach log file if it exists
if os.path.exists(log_path):
    with open(log_path, 'rb') as attachment:
        part = MIMEBase('application', 'octet-stream')
        part.set_payload(attachment.read())
    
    encoders.encode_base64(part)
    part.add_header(
        'Content-Disposition',
        f'attachment; filename= {log_filename}'
    )
    msg.attach(part)
    print(f"Attached log file: {log_filename}")
else:
    print(f"Log file not found: {log_path}")

try:
    with smtplib.SMTP_SSL('smtp.gmail.com', 465) as smtp:
        smtp.login(EMAIL_ADDRESS, EMAIL_PASSWORD)
        smtp.send_message(msg)
    print("Email sent successfully.")
except Exception as e:
    print("Failed to send email:", e)
