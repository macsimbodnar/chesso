import subprocess,sys,re
sys.path.insert(0,'.')
from comment_lexer import strip_comments
def toks(s):
    # compare as whitespace-free text: g++ -E -P rewrites spacing and collapses blank lines
    return re.sub(r'\s+','',s)
for f in sys.argv[1:]:
    t=open('/home/max/ws/chesso/'+f,encoding='utf-8').read()
    mine=strip_comments(f,t)
    # -fpreprocessed strips comments, -dD keeps defines, nothing is expanded
    g=subprocess.run(['g++','-fpreprocessed','-dD','-E','-P','-x','c++',f'/home/max/ws/chesso/{f}'],capture_output=True,text=True)
    a,b=toks(mine),toks(g.stdout)
    print(f,'lexer',len(a),'g++',len(b),'identical' if a==b else 'DIFFER', g.returncode)
    if a!=b:
        i=next((i for i,(x,y) in enumerate(zip(a,b)) if x!=y),min(len(a),len(b)))
        print(' first diff at',i,repr(a[i-60:i+60]),repr(b[i-60:i+60]))
