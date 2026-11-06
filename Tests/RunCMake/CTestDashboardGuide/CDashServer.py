"""Local stand-in for a CDash server.

Each submitted file is recorded as one JSON object per line in the log file.

The server stops when it receives a GET request for /shutdown, or after it
has been idle for --idle-timeout seconds.
"""

import argparse
import hashlib
import json
import os
import subprocess
import sys
import textwrap
import threading
import time
import urllib.parse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

args = None
log_lock = threading.Lock()
last_request = time.monotonic()
stop = threading.Event()


class CDashHandler(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def do_PUT(self):
        global last_request
        last_request = time.monotonic()
        length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(length)
        url = urllib.parse.urlsplit(self.path)
        query = urllib.parse.parse_qs(url.query)
        record = {
            "path": url.path,
            "project": query.get("project", [""])[0],
            "build": query.get("build", [""])[0],
            "file": query.get("FileName", [""])[0],
            "authorization": self.headers.get("Authorization", ""),
        }
        with log_lock:
            with open(args.log_file, "a", encoding="utf-8") as f:
                f.write(json.dumps(record) + "\n")
        md5 = hashlib.md5(body).hexdigest()
        self.reply(
            textwrap.dedent(f"""
            <cdash version=\"5.0\">
              <status>OK</status>
              <message></message>
              <md5>{md5}</md5>
              <buildId>1</buildId>
            </cdash>
        """)
        )

    def do_GET(self):
        global last_request
        last_request = time.monotonic()
        if self.path == "/ping":
            self.reply("")
        elif self.path == "/shutdown":
            self.reply("")
            stop.set()
        else:
            self.send_error(404)

    def reply(self, text):
        data = text.encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "text/xml")
        self.send_header("Content-Length", str(len(data)))
        # Close each connection so that curl does not close it while cleaning
        # up, which some versions log to stderr.
        self.send_header("Connection", "close")
        self.end_headers()
        self.wfile.write(data)

    def log_message(self, format, *args):
        pass


def serve():
    local_address = "127.0.0.1"
    httpd = ThreadingHTTPServer((local_address, 0), CDashHandler)
    thread = threading.Thread(target=httpd.serve_forever, daemon=True)
    thread.start()

    # Write the URL file atomically: the test polls for it to exist, so it must
    # never see it half written.
    url = f"http://{local_address}:{httpd.server_address[1]}"
    tmp_url_path = f"{args.url_file}.tmp"
    with open(tmp_url_path, "w", encoding="utf-8") as f:
        f.write(url)
    os.replace(tmp_url_path, args.url_file)

    while not stop.wait(1):
        if time.monotonic() - last_request > args.idle_timeout:
            break
    httpd.shutdown()
    httpd.server_close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--url-file", required=True, help="file to write the server URL to"
    )
    parser.add_argument(
        "--log-file", required=True, help="file to record submissions in"
    )
    parser.add_argument(
        "--idle-timeout",
        type=float,
        default=600,
        help="seconds without requests before exiting",
    )
    parser.add_argument("--subprocess", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args()

    if args.subprocess:
        serve()
    else:
        # Run the server in the background and return immediately.
        subprocess.Popen(
            [sys.executable] + sys.argv + ["--subprocess"],
            stdin=subprocess.DEVNULL,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
