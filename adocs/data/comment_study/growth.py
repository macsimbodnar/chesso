# Comment bytes at dated commits on achesso, same lexer. Light: one git show per file.
import subprocess,sys
sys.path.insert(0,'.')
from comment_lexer import lex,language_of
REPO='/home/max/ws/chesso'
def g(*a): return subprocess.run(['git','-C',REPO,*a],capture_output=True).stdout.decode('utf-8','replace')
dates=['2026-08-08','2026-08-15','2026-09-01','2026-09-15','2026-10-01','2026-10-15','2026-10-31T23:00']
print('date sha src_code_lines src_comment_bytes src_bytes share% tests_comment tests_bytes tools_comment tools_bytes')
for d in dates:
    sha=g('rev-list','-1',f'--before={d}','5954c12').strip()
    res={}
    for area in ('src','tests','tools'):
        files=g('ls-tree','-r','--name-only',sha,area).split('\n')
        cb=by=cl=0
        for f in files:
            if not language_of(f) or f.startswith('tests/third_party') or f.startswith('tests/json') or f.startswith('tests/doctest'): continue
            t=g('show',f'{sha}:{f}')
            try: cs,code=lex(f,t)
            except Exception: continue
            cb+=sum(len(t[c.start_off:c.end_off].encode()) for c in cs if c.kind!='shebang'); by+=len(t.encode()); cl+=len(code)
        res[area]=(cb,by,cl)
    print(d[:10],sha[:7],res['src'][2],res['src'][0],res['src'][1],round(100*res['src'][0]/max(1,res['src'][1]),1),res['tests'][0],res['tests'][1],res['tools'][0],res['tools'][1])
