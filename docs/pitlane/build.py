import base64, os, re, sys
S = os.path.dirname(os.path.abspath(__file__)).replace(chr(92), '/')
NL = chr(10)
src = open(S + '/../robot-guide.html', encoding='utf-8', newline='').read().replace('\r\n', NL)

def face(w, f):
    b = base64.b64encode(open(f'{S}/fonts/{f}', 'rb').read()).decode()
    return '@font-face{font-family:"IBM Plex Sans Thai";font-style:normal;font-weight:%d;font-display:swap;src:url(data:font/ttf;base64,%s) format("truetype")}' % (w, b) + NL

fonts = '/* IBM Plex Sans Thai: SIL Open Font License, Google Fonts */' + NL + face(400, 'IBMPlexSansThai-400.ttf') + face(700, 'IBMPlexSansThai-700.ttf')
rd = lambda n: open(f'{S}/{n}', encoding='utf-8').read()
a = src.index('<style>') + len('<style>' + NL)
b = src.index('</style>')

def make(layout):
    css = rd('new.css') + (rd('new2.css') + rd('new3.css') if layout else '')
    out = src[:a] + fonts + css + src[b:]
    out = out.replace(' style="background:#4338ca;color:#fff"', '').replace('color:var(--ink-3);font-size:16px', 'color:var(--fg3);font-size:16px')
    out = re.sub(r'<meta name="theme-color"[^>]*>', '<meta name="theme-color" content="#0d0f12">', out)
    if layout:
        out = out.replace('</body>', '<script>' + NL + rd('browser.js') + '</script>' + NL + '</body>')
    return out.replace(NL, '\r\n')

os.makedirs(f'{S}/dist', exist_ok=True)
open(f'{S}/dist/index.html', 'w', encoding='utf-8', newline='').write(make(True))
print('ok')
