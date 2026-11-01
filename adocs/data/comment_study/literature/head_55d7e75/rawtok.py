# Does Apple clang's raw token dump agree with the study's lexer strip, over every live C++ file at HEAD?
import sys, subprocess, tempfile, os, hashlib, re, time
sys.path.insert(0,'/Users/max/ws/chesso/adocs/data/comment_study')
import count_comments as cc
from comment_lexer import language_of, strip_comments
REPO='/Users/max/ws/chesso'
files=subprocess.run(['git','-C',REPO,'ls-tree','-r','--name-only','HEAD'],capture_output=True,text=True).stdout.split('\n')
tok_re=re.compile(r"^(\S+) '")
def toks(path):
    p=subprocess.run(['/usr/bin/clang++','-x','c++','-fsyntax-only','-Xclang','-dump-raw-tokens',path],capture_output=True)
    out=p.stderr.decode('utf-8','replace')
    res=[];cur=None
    # records end with Loc=<...>; a token text may span lines
    for rec in re.split(r"\tLoc=<[^>]*>\n", out):
        rec=rec.strip('\n')
        if not rec: continue
        m=re.match(r"^(\S+) '(.*)'\t?( \[.*\])?$",rec,re.S)
        if not m: 
            res.append(('??',rec[:60])); continue
        k,t=m.group(1),m.group(2)
        if k=='comment' or (k=='unknown' and t.strip()==''): continue
        res.append((k,t))
    return res
same=diff=0;t0=time.time();bad=[]
with tempfile.TemporaryDirectory() as d:
    for f in files:
        if not f or f.startswith(cc.EXCLUDE_PREFIX): continue
        a=cc.area_of(f)
        if a in (None,'adocs_data'): continue
        if language_of(f)!='cpp': continue
        text=subprocess.run(['git','-C',REPO,'show',f'HEAD:{f}'],capture_output=True).stdout.decode()
        ext=os.path.splitext(f)[1] or '.cpp'
        p1=os.path.join(d,'a'+ext);p2=os.path.join(d,'b'+ext)
        open(p1,'w').write(text); open(p2,'w').write(strip_comments(f,text))
        ta=toks(p1);tb=toks(p2)
        if ta==tb and ta: same+=1
        else:
            diff+=1;bad.append((f,len(ta),len(tb)))
print('C++ files same',same,'diff',diff,'secs',round(time.time()-t0,1));print(bad[:10])
# control: one code token changed must differ
with tempfile.TemporaryDirectory() as d:
    p=os.path.join(d,'c.cpp');open(p,'w').write('int a = 1; // x\n#if 0\nint z = 3;\n#endif\n');x=toks(p)
    open(p,'w').write('int a = 1; // y\n#if 0\nint z = 4;\n#endif\n');y=toks(p)
    print('control inactive-branch change differs:',x!=y)
    open(p,'w').write('int a = 1;\n\n\n#if 0\nint z = 3;\n#endif\n');y=toks(p)
    print('control comment-only edit same:',x==y)
