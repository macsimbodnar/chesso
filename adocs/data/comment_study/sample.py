#!/usr/bin/env python3
"""Question 2: stratified sample of comment blocks, area x size band, fixed seed.
Writes sample.jsonl and sample_dump_<area>.txt with the block, what follows it,
and any identifier the comment names that appears nowhere in the tree's code."""
import json, random, re, subprocess, sys
from collections import defaultdict
from pathlib import Path
sys.path.insert(0,'.')
from comment_lexer import strip_comments, language_of
SEED=20261031
COMMIT='5954c12'
REPO='/home/max/ws/chesso'
ALLOC={('src','xs'):6,('src','s'):16,('src','m'):18,('src','l'):10,('src','xl'):5,
('tests','xs'):5,('tests','s'):16,('tests','m'):14,('tests','l'):8,('tests','xl'):3,
('tools','xs'):4,('tools','s'):10,('tools','m'):8,('tools','l'):4,('tools','xl'):2,
('adocs_data','xs'):3,('adocs_data','s'):5,('adocs_data','m'):5,('adocs_data','l'):4,('adocs_data','xl'):6,
('root+cmake','xs'):1,('root+cmake','s'):2,('root+cmake','m'):4,('root+cmake','l'):3,('root+cmake','xl'):1}
bands=[(0,120,'xs'),(120,400,'s'),(400,1200,'m'),(1200,3000,'l'),(3000,10**9,'xl')]
def band(b):
    for lo,hi,n in bands:
        if lo<=b<hi: return n
def show(p):
    return subprocess.run(['git','-C',REPO,'show',f'{COMMIT}:{p}'],capture_output=True,check=True).stdout.decode()
B=[json.loads(l) for l in open('blocks.jsonl')]
strata=defaultdict(list)
for b in B:
    a='root+cmake' if b['area'] in('cmake','root') else b['area']
    b['stratum_area']=a; b['band']=band(b['comment_bytes'])
    strata[(a,b['band'])].append(b)
rng=random.Random(SEED)
sample=[]
for k in sorted(ALLOC):
    pop=sorted(strata[k],key=lambda b:(b['file'],b['start_line']))
    chosen=rng.sample(pop,min(ALLOC[k],len(pop)))
    for c in chosen:
        c['stratum_blocks']=len(pop); c['stratum_bytes']=sum(x['comment_bytes'] for x in pop)
    sample+=chosen
sample.sort(key=lambda b:(b['stratum_area'],b['file'],b['start_line']))
for i,b in enumerate(sample): b['id']=f"B{i+1:03d}"
# every word that occurs in stripped code of the census tree
words=set()
wre=re.compile(r'[A-Za-z_][A-Za-z0-9_]{3,}')
files=sorted({l.split('\t')[0] for l in open('files.tsv')}-{'file'})
for f in files:
    if f.startswith('adocs/data/'): continue
    try:
        words.update(wre.findall(strip_comments(f,show(f))))
    except Exception as e:
        print('strip failed',f,e)
idre=re.compile(r'`([^`\n]{2,60})`|\b([a-z]+_[a-z0-9_]+)\b|\b([a-z]+[A-Z][A-Za-z0-9]+)\b')
cache={}
out=defaultdict(list)
for b in sample:
    t=cache.setdefault(b['file'],show(b['file']))
    txt=t[b['start_off']:b['end_off']]
    lines=t.split('\n')
    after=lines[b['end_line']:b['end_line']+5]
    if b['trailing']:
        ctx=lines[b['start_line']-1]
    else:
        ctx='\n'.join(after)
    missing=set()
    for m in idre.finditer(txt):
        tok=m.group(1) or m.group(2) or m.group(3)
        for w in wre.findall(tok):
            if w not in words and ('_' in w or any(ch.isupper() for ch in w[1:])):
                missing.add(w)
    b['missing_ids']=sorted(missing)
    b['text_bytes']=len(txt.encode())
    limit=2500 if (b['area']=='adocs_data' and b['band']=='xl') else 10**9
    show_txt=txt if len(txt)<=limit else txt[:limit]+f"\n...[truncated, {len(txt)} chars]"
    hdr=f"=== {b['id']} {b['file']}:{b['start_line']}-{b['end_line']} {b['comment_bytes']}B {b['band']} {b['kind']}{' trailing' if b['trailing'] else ''}"
    body=hdr+"\n"+show_txt+"\n--- code:\n"+ctx[:320]+"\n"
    if missing: body+="--- not in code: "+' '.join(sorted(missing))+"\n"
    out[b['stratum_area']].append(body)
with open('sample.jsonl','w') as f:
    for b in sample: f.write(json.dumps(b)+'\n')
for a,v in out.items():
    Path(f'sample_dump_{a}.txt').write_text('\n'.join(v))
print(len(sample),{a:len(v) for a,v in out.items()}, sum(b['comment_bytes'] for b in sample))
