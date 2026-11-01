#!/usr/bin/env python3
"""How much comment is log-shaped: long blocks dense in step and decision ids. A mechanical
selector for a narrow cleanup, validated against the hand classes in the sample."""
import json,re,subprocess,csv
from collections import Counter
C='5954c12';R='/home/max/ws/chesso'
cache={}
def blob(p):
    if p not in cache: cache[p]=subprocess.run(['git','-C',R,'show',f'{C}:{p}'],capture_output=True).stdout.decode()
    return cache[p]
ID=re.compile(r'\bS\d{3}\b|\bDEC-\d{3}\b')
def density(text,n):
    return len(set(ID.findall(text)))
S=[json.loads(l) for l in open('sample_classified.jsonl')]
def is_log(ids,b):
    return b>=1200 and ids>=6 and ids/(b/1000)>=1.5
# validate
live=[b for b in S if b['stratum_area']!='adocs_data']
A=[];N=[]
for b in live:
    t=blob(b['file'])[b['start_off']:b['end_off']]
    ids=density(t,b['comment_bytes'])
    (A if is_log(ids,b['comment_bytes']) else N).append(b)
def wh(bs):
    tot=sum(x['comment_bytes'] for x in bs) or 1
    return {c:round(100*sum(x['frac'].get(c,0)*x['comment_bytes'] for x in bs)/tot) for c in 'CWRHTX'},len(bs),tot
print('sample, log-shaped:',wh(A),[x['id'] for x in A])
print('sample, the rest  :',wh(N))
print('stale in log-shaped',sum(x['stale'] for x in A),'of',len(A),'; rest',sum(x['stale'] for x in N),'of',len(N))
B=[json.loads(l) for l in open('blocks.jsonl')]
files=Counter();by=Counter();n=0;tot=0;live_tot=0
rows=[]
for b in B:
    if b['area']=='adocs_data': continue
    live_tot+=b['comment_bytes']
    t=blob(b['file'])[b['start_off']:b['end_off']]
    ids=density(t,b['comment_bytes'])
    if is_log(ids,b['comment_bytes']):
        n+=1;tot+=b['comment_bytes'];files[b['file']]+=b['comment_bytes'];by[b['area']]+=b['comment_bytes'];rows.append((b['comment_bytes'],ids,b['file'],b['start_line']))
print('population live code: log-shaped blocks',n,'bytes',tot,'share of live comment bytes %.1f%%'%(100*tot/live_tot),dict(by))
print('by file:',files.most_common(14))
rows.sort(reverse=True)
print('largest:',rows[:12])
json.dump(rows,open('log_shaped.json','w'))
read=json.load(open('read_files.json'))
rc={k.split('|')[0]:v for k,v in read.items() if k.endswith('|comment')}
census={r['file']:r for r in csv.DictReader(open('files.tsv'),delimiter='\t')}
trc=sum(rc.values())
# traffic attributable: each file's read share x the block's share of that file's comment bytes
tr=sum(rc.get(f,0)*bytes_/max(1,int(census[f]['comment_bytes'])) for f,bytes_ in files.items())
print('estimated share of comment reads falling on these blocks %.1f%% (uniform within a file)'%(100*tr/trc))
