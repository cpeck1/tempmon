from freezers.models import Freezer
from server.settings import ADMINS, SERVER_EMAIL, SERVER_EMAIL_PW
from server.settings import SERVER_EMAIL_SMTP_SERVER, SERVER_EMAIL_SMTP_PORT
from server.settings import SERVER_EMAIL_SUBJECT_TEMPLATE
from server.settings import SERVER_EMAIL_BODY_TEMPLATE

import smtplib

def validate_status(Freezer):
    '''
    Checks the given status message for either "WARNING" or "ERROR". If it 
    contains either of these strings, email send an email to admins.

    All specifications for message can be found in server/settings.py
    '''
    if ("WARNING" in Freezer.status) or ("ERROR" in Freezer.status):
        SMTP_SERVER = SERVER_EMAIL_SMTP_SERVER
        SMTP_PORT = SERVER_EMAIL_SMTP_PORT

        sender = SERVER_EMAIL
        subject = SERVER_EMAIL_SUBJECT_TEMPLATE % Freezer.id
        
        body = SERVER_EMAIL_BODY_TEMPLATE
        body = body % (Freezer.id, Freezer.status, Freezer.temperature,
                       Freezer.last_updated)
        body = "" + body + ""
        session = smtplib.SMTP(SMTP_SERVER, SMTP_PORT)
        session.ehlo()
        session.starttls()
        session.ehlo
        session.login(sender, SERVER_EMAIL_PW)
        for admin in ADMINS:
            recipient = admin[1]
            headers = ["From: " + sender,
                       "Subject: " + subject,
                       "To: " + recipient,
                       "MIME-Version: 1.0",
                       "Content-Type: text/html"]
            headers = "\r\n".join(headers)
            session.sendmail(sender, recipient, headers + "\r\n\r\n" + body)
        session.quit()
        
