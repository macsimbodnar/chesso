# Which live comment blocks at HEAD overlap the old side of S101's paused diff, code files only. Approximate:
# line numbers are at the diff's base (REF 4e34e61), so compare by the file at that base.
import re, subprocess, sys, collections
sys.path.insert(0,'/Users/max/ws/chesso/adocs/data/comment_study')
import count_comments as cc
from comment_lexer import lex
R='/Users/max/ws/chesso'
d=open(f'{R}/adocs/data/S101_phase2_pause.diff',errors='replace').read()
base='4e34e61'
cur=None;h=collections.defaultdict(list)
for line in d.split('\n'):
    if line.startswith('diff --git'):
        cur=line.split(' b/')[-1]
    m=re.match(r'^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@',line)
    if m and cur:
        s=int(m.group(1));n=int(m.group(2) or 1);h[cur].append((s,s+max(n,1)-1))
tot=0;totb=0
for f,hs in h.items():
    if not (f.startswith('src/') or f.startswith('tests/')): continue
    t=subprocess.run(['git','-C',R,'show',f'{base}:{f}'],capture_output=True).stdout.decode()
    bl=cc.blockify(f,t,lex(f,t)[0])
    ov=[b for b in bl if any(not(b['end_line']<a or b['start_line']>z) for a,z in hs)]
    fb=sum(b['comment_bytes'] for b in bl)
    bytes_=sum(b['comment_bytes'] for b in ov)
    print(f,'hunks',len(hs),'blocks_overlap',len(ov),'bytes',bytes_,'of',fb)
    tot+=len(ov);totb+=bytes_
print('total overlap blocks',tot,'bytes',totb)
