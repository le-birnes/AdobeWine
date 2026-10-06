#!/usr/bin/env python3
"""Local HTTP server for asyncrecv.c / asyncflow.c: POST /ingest answers after 3 s,
GET /r redirects to /ok, GET /ok answers 'hello'. Usage: testserver.py [port]"""
import http.server, sys, time
class H(http.server.BaseHTTPRequestHandler):
    protocol_version = 'HTTP/1.1'
    def reply(self, code, body=b'', extra=()):
        self.send_response(code)
        for k, v in extra: self.send_header(k, v)
        self.send_header('Content-Length', str(len(body))); self.end_headers(); self.wfile.write(body)
    def do_POST(self):
        self.rfile.read(int(self.headers.get('Content-Length', 0))); time.sleep(3); self.reply(200, b'ok')
    def do_GET(self):
        if self.path == '/r': self.reply(302, b'', [('Location', '/ok')])
        else: self.reply(200, b'hello')
    def log_message(self, *a): pass
http.server.ThreadingHTTPServer(('127.0.0.1', int(sys.argv[1]) if len(sys.argv) > 1 else 18765), H).serve_forever()
