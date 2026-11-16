from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import os
import subprocess
import sys
import threading


class CDashHandler(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def setup(self):
        super().setup()
        # Bound reads from clients that connect but do not finish a request.
        self.connection.settimeout(30)

    def log_message(self, format, *args):
        pass

    def respond(self, body):
        self.server.requests.append(self.command)
        self.rfile.read(int(self.headers.get("Content-Length", 0)))
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Connection", "close")
        self.end_headers()
        self.wfile.write(body)
        self.close_connection = True

    def do_POST(self):
        if self.server.case == "CDashUploadInvalidRequest":
            self.respond(b"not JSON")
        elif self.server.case == "CDashUploadExists":
            self.respond(b'{"status":0,"buildid":"123","datafilesmd5":[1]}')
        else:
            self.respond(b'{"status":0,"buildid":"123","datafilesmd5":[0]}')

    def do_PUT(self):
        if self.server.case == "CDashUploadInvalidResponse":
            self.respond(b"not JSON")
        else:
            self.respond(b'{"status":0}')


def main():
    case = sys.argv[1]
    with ThreadingHTTPServer(("127.0.0.1", 0), CDashHandler) as server:
        server.case = case
        server.requests = []
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        env = os.environ.copy()
        env["CTEST_SUBMIT_URL"] = f"http://127.0.0.1:{server.server_port}/submit.php"
        env["no_proxy"] = "127.0.0.1"
        env["NO_PROXY"] = "127.0.0.1"
        try:
            result = subprocess.run(
                sys.argv[2:], env=env, stdin=subprocess.DEVNULL, timeout=60
            )
        finally:
            server.shutdown()
            thread.join()

    expected = ["POST"]
    if case in ("CDashUploadSuccess", "CDashUploadInvalidResponse"):
        expected.append("PUT")
    if server.requests != expected:
        sys.exit(f"Expected requests {expected}, got {server.requests}")
    return result.returncode


if __name__ == "__main__":
    sys.exit(main())
