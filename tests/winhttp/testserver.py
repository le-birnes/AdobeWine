#!/usr/bin/env python3
"""Local HTTP server for the WinHTTP tests in this directory. Usage: testserver.py [port]
asyncrecv.c / asyncflow.c: POST /ingest answers after 3 s, GET /r redirects to /ok, GET /ok answers 'hello'.
readavail.c (0039): every reply carries X-Conn, a number per TCP connection (keep-alive reuse check);
  /chunk     chunked '123456789', the data chunk and the last chunk in one write
  /gzchunk   gzip body (GZTEXT), chunked, all chunks and the last chunk in one write
  /gzlen     gzip body (GZTEXT) with the compressed Content-Length
  /slow      4000 bytes with Content-Length: 100 bytes, then the rest 1.5 s later"""
import gzip, http.server, itertools, sys, time
GZTEXT = b''.join(b'line %04d of the decompressed body\n' % i for i in range(60))
conns = itertools.count(1)
class H(http.server.BaseHTTPRequestHandler):
    protocol_version = 'HTTP/1.1'
    def setup(self):
        super().setup(); self.conn = next(conns)
    def head(self, code, extra):
        self.send_response(code)
        self.send_header('X-Conn', str(self.conn))
        for k, v in extra: self.send_header(k, v)
        self.end_headers()
    def reply(self, code, body=b'', extra=()):
        self.head(code, list(extra) + [('Content-Length', str(len(body)))]); self.wfile.write(body)
    def chunked(self, body, extra=()):
        self.head(200, list(extra) + [('Transfer-Encoding', 'chunked')])
        data = b''.join(b'%x\r\n%s\r\n' % (len(body[i:i + 1000]), body[i:i + 1000]) for i in range(0, len(body), 1000))
        self.wfile.write(data + b'0\r\n\r\n')
    def do_POST(self):
        self.rfile.read(int(self.headers.get('Content-Length', 0))); time.sleep(3); self.reply(200, b'ok')
    def do_GET(self):
        if self.path == '/r': self.reply(302, b'', [('Location', '/ok')])
        elif self.path == '/chunk': self.chunked(b'123456789')
        elif self.path == '/gzchunk': self.chunked(gzip.compress(GZTEXT), [('Content-Encoding', 'gzip')])
        elif self.path == '/gzlen': self.reply(200, gzip.compress(GZTEXT), [('Content-Encoding', 'gzip')])
        elif self.path == '/slow':
            body = bytes(48 + i % 10 for i in range(4000))
            self.head(200, [('Content-Length', str(len(body)))])
            self.wfile.write(body[:100]); self.wfile.flush(); time.sleep(1.5); self.wfile.write(body[100:])
        else: self.reply(200, b'hello')
    def log_message(self, *a): pass
http.server.ThreadingHTTPServer(('127.0.0.1', int(sys.argv[1]) if len(sys.argv) > 1 else 18765), H).serve_forever()
