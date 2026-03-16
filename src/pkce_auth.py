import hashlib, base64, secrets, urllib.parse, webbrowser, json
from http.server import HTTPServer, BaseHTTPRequestHandler
import urllib.request
from dotenv import load_dotenv
import os

load_dotenv()
CLIENT_ID = os.getenv("SPOTIFY_CLIENT_ID")
REDIRECT_URI = "http://127.0.0.1:8080/callback"
SCOPES = "user-top-read user-read-currently-playing user-library-read"

def build_pkce():
    verifier = secrets.token_urlsafe(32)
    challenge = base64.urlsafe_b64encode(
        hashlib.sha256(verifier.encode()).digest()
    ).rstrip(b'=').decode()
    return verifier, challenge

def build_auth_url(challenge):
    params = urllib.parse.urlencode({
        "response_type": "code",
        "client_id": CLIENT_ID,
        "scope": SCOPES,
        "redirect_uri": REDIRECT_URI,
        "code_challenge_method": "S256",
        "code_challenge": challenge
    })
    return f"https://accounts.spotify.com/authorize?{params}"

def make_handler(verifier):
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, format, *args):
            pass

        def do_GET(self):
            code = urllib.parse.parse_qs(
                urllib.parse.urlparse(self.path).query
            )["code"][0]

            data = urllib.parse.urlencode({
                "grant_type": "authorization_code",
                "code": code,
                "redirect_uri": REDIRECT_URI,
                "client_id": CLIENT_ID,
                "code_verifier": verifier
            }).encode()

            req = urllib.request.Request(
                "https://accounts.spotify.com/api/token",
                data=data,
                headers={"Content-Type": "application/x-www-form-urlencoded"}
            )
            resp = json.loads(urllib.request.urlopen(req).read())

            with open("tokens.json", "w") as f:
                json.dump({
                    "access_token": resp["access_token"],
                    "refresh_token": resp["refresh_token"],
                    "expires_in": resp["expires_in"]
                }, f)

            self.send_response(200)
            self.end_headers()
            self.wfile.write(b"<html><body><h2>Success!! Please close this tab :)</h2></body></html>")
    return Handler

def run_auth():
    verifier, challenge = build_pkce()
    url = build_auth_url(challenge)
    webbrowser.open(url)
    HTTPServer(("127.0.0.1", 8080), make_handler(verifier)).handle_request()

if __name__ == "__main__":
    #print("\nOpening browser for authentication...")

    run_auth()

    try:
        with open("tokens.json") as f:
            tokens = json.load(f)
        #print("\nAuth successful! tokens.json written.")
        with open("output.txt", "w") as file:
            file.write("all good!")

    
    except FileNotFoundError:
        #print("\nAuth failed — tokens.json was not written.")
        with open("output.txt", "w") as file:
            file.write("failed to authenticate")
