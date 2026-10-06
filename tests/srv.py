import http.server, urllib.parse, sys, os
os.chdir(os.path.dirname(os.path.abspath(__file__)))
class H(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path.startswith('/r?'):
            open('results.txt','a').write(urllib.parse.unquote(self.path[3:])+'\n')
            self.send_response(204); self.end_headers(); return
        super().do_GET()
    def log_message(self,*a): pass
http.server.HTTPServer(('127.0.0.1',8765),H).serve_forever()
