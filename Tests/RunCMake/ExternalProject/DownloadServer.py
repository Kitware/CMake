from http.server import HTTPServer, BaseHTTPRequestHandler
import argparse
import time
import subprocess
import sys
import os
import threading
args = None
outerthread = None

barrier = threading.Barrier(2)

class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        barrier.wait()
        self.send_response(200)
        self.end_headers()
        data = b'D'

        if args.speed_limit:
            slow_deadline = time.time()+args.limit_duration

            while time.time() < slow_deadline:
                self.wfile.write(data)
                if args.speed_limit:
                    time.sleep(1.1)

        data = data * 100
        self.wfile.write(data)
        self.close_connection = True

def runServer(fileName):
    httpd = HTTPServer(('localhost', 0), SimpleHTTPRequestHandler)
    with open(fileName,"w") as f:
        f.write('http://localhost:{}/test'.format(httpd.socket.getsockname()[1]))
    httpd.handle_request()
    os.remove(fileName)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--speed_limit', help='transfer rate limitation', action='store_true',default=False)
    parser.add_argument('--limit_duration', help='duration of the transfer rate limitation',default=1, type=float)
    parser.add_argument('--file', help='file to write the url to connect to')
    parser.add_argument('--subprocess', action='store_true')
    args = parser.parse_args()
    if not args.subprocess:
        subprocess.Popen([sys.executable]+sys.argv+['--subprocess'],stdin=subprocess.DEVNULL, stderr=subprocess.DEVNULL,stdout=subprocess.DEVNULL)
    else:
        serverThread = threading.Thread(target=runServer,args=(args.file,))
        serverThread.daemon = True
        serverThread.start()
        barrier.wait(60)
        serverThread.join(20)
