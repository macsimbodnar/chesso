import sys, subprocess, re, json, collections
sys.path.insert(0,'/Users/max/ws/chesso/adocs/data/comment_study')
R='/Users/max/ws/chesso'
rows=json.load(open('blocks_head.json'))
ID=re.compile(r'\bS\d{3}\b|\bDEC-\d{3}\b')
cache={}
def blob(p):
    if p not in cache: cache[p]=subprocess.run(['git','-C',R,'show',f'HEAD:{p}'],capture_output=True).stdout.decode()
    return cache[p]
log=[];files=collections.Counter();area=collections.Counter()
for f,a,b in rows:
    t=blob(f)[b['start_off']:b['end_off']]
    ids=len(set(ID.findall(t)))
    if b['comment_bytes']>=1200 and ids>=6 and ids/(b['comment_bytes']/1000)>=1.5:
        log.append((f,b['start_line'],b['comment_bytes'],ids));files[f]+=b['comment_bytes'];area[a]+=b['comment_bytes']
print('log-shaped at HEAD',len(log),sum(x[2] for x in log),'files',len(files));print(area);print(files.most_common(8))
# refined forward references to closed steps
import os
done={p[:4] for p in os.listdir(f'{R}/adocs/plan_done') if re.match(r'S\d{3}',p)}
FWD=re.compile(r"\b(S\d{3})\s+(?:owns|owes|will|is pending|is open|takes this|has yet|still owes)\b|\b(?:until|once|when|waits? (?:for|on))\s+(S\d{3})\s+(?:lands?|closes?|ships?|is done)\b|\b(?:an? H[01] (?:on|for)|pending)\s+(S\d{3})\b")
cnt=collections.Counter();ex=[]
for f in sorted({r[0] for r in rows}):
    from comment_lexer import lex
    t=blob(f)
    for m in lex(f,t)[0]:
        s=t[m.start_off:m.end_off]
        for x in FWD.finditer(s):
            sid=x.group(1) or x.group(2) or x.group(3)
            k='closed' if sid in done else 'open'
            cnt[k]+=1
            if k=='closed': ex.append((f,m.start_line,re.sub(r'\s+',' ',s[max(0,x.start()-30):x.end()+35])))
print(cnt);print(len({(e[0],e[1]) for e in ex}))
for e in ex[:14]:print(e)
