#!/usr/bin/env python3
import os
import sys
import random
import string

COOKIE_NAME = "sessionid"

def send_headers(headers):
    for h in headers:
        sys.stdout.write(h + "\r\n")
    sys.stdout.write("\r\n")

def parse_cookies():
    raw = os.environ.get("HTTP_COOKIE", "")
    cookies = {}
    for part in raw.split(';'):
        if '=' in part:
            k, v = part.strip().split('=', 1)
            cookies[k] = v
    return cookies

def generate_session_id(length=32):
    chars = string.ascii_letters + string.digits
    return ''.join(random.choice(chars) for _ in range(length))

def main():
    cookies = parse_cookies()
    headers = []

    # Session ID handling
    sessionid = cookies.get(COOKIE_NAME)
    if not sessionid:
        sessionid = generate_session_id()
        headers.append(f"Set-Cookie: {COOKIE_NAME}={sessionid}; Path=/; Max-Age=86400; HttpOnly")

    # Visit tracking
    visit_cookie = f"visits_{sessionid}"
    visits = int(cookies.get(visit_cookie, "0")) + 1
    headers.append(f"Set-Cookie: {visit_cookie}={visits}; Path=/; Max-Age=86400")

    headers.append("Content-Type: text/html")
    send_headers(headers)

    # Response
    print("<html><body>")
    print("<h2>Visit Tracker (Cookie-based only)</h2>")
    print(f"<p>Session ID: <code>{sessionid}</code></p>")
    print(f"<p>Visit Count (for this session): <strong>{visits}</strong></p>")

    print("<h3>All Cookies Sent By Browser:</h3><ul>")
    for k, v in cookies.items():
        print(f"<li>{k} = {v}</li>")
    print("</ul>")

    print("</body></html>")

if __name__ == "__main__":
    main()