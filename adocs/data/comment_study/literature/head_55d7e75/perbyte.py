# Regenerates the study's sample in scratch (nothing written to the repo) and reads dead claims per byte.
import sys, json, random, csv, os, math
from pathlib import Path
from collections import defaultdict
sys.path.insert(0,'/Users/max/ws/chesso/adocs/data/comment_study')
import count_comments as cc
S=Path('/private/tmp/claude-501/-Users-max-ws-chesso-build/8e195c86-e693-45ac-b65e-6be5437b5a74/scratchpad/regen'); S.mkdir(exist_ok=True)
cc.OUT=S; cc.REPO='/Users/max/ws/chesso'; cc.COMMIT='5954c12'
cc.main()
ALLOC={('src','xs'):6,('src','s'):16,('src','m'):18,('src','l'):10,('src','xl'):5,
('tests','xs'):5,('tests','s'):16,('tests','m'):14,('tests','l'):8,('tests','xl'):3,
('tools','xs'):4,('tools','s'):10,('tools','m'):8,('tools','l'):4,('tools','xl'):2,
('adocs_data','xs'):3,('adocs_data','s'):5,('adocs_data','m'):5,('adocs_data','l'):4,('adocs_data','xl'):6,
('root+cmake','xs'):1,('root+cmake','s'):2,('root+cmake','m'):4,('root+cmake','l'):3,('root+cmake','xl'):1}
bands=[(0,120,'xs'),(120,400,'s'),(400,1200,'m'),(1200,3000,'l'),(3000,10**9,'xl')]
def band(b):
    for lo,hi,n in bands:
        if lo<=b<hi: return n
B=[json.loads(l) for l in open(S/'blocks.jsonl')]
strata=defaultdict(list)
for b in B:
    a='root+cmake' if b['area'] in('cmake','root') else b['area']
    b['stratum_area']=a; b['band']=band(b['comment_bytes'])
    strata[(a,b['band'])].append(b)
rng=random.Random(20261031); sample=[]
for k in sorted(ALLOC):
    pop=sorted(strata[k],key=lambda b:(b['file'],b['start_line']))
    chosen=rng.sample(pop,min(ALLOC[k],len(pop)))
    for c in chosen:
        c['stratum_blocks']=len(pop); c['stratum_bytes']=sum(x['comment_bytes'] for x in pop)
    sample+=chosen
sample.sort(key=lambda b:(b['stratum_area'],b['file'],b['start_line']))
for i,b in enumerate(sample): b['id']=f"B{i+1:03d}"
cl={r['id']:r for r in csv.DictReader(open('/Users/max/ws/chesso/adocs/data/comment_study/classified.tsv'),delimiter='\t')}
print('sample',len(sample),'classified',len(cl))
for b in sample:
    r=cl[b['id']]; b['stale']=int(r['stale']); b['note']=r['note']
for b in sample:
    if b['stale']: print(b['id'],b['file'],b['start_line'],b['comment_bytes'],b['note'][:60])
live=[b for b in sample if b['stratum_area']!='adocs_data']
L=[b for b in live if b['comment_bytes']>=1200]; N=[b for b in live if b['comment_bytes']<1200]
for name,s in (('>=1200',L),('<1200',N)):
    k=sum(b['stale'] for b in s); by=sum(b['comment_bytes'] for b in s)
    print(name,'blocks',len(s),'stale',k,'bytes',by,'mean',round(by/len(s)),'stale per 10 KB',round(k/by*1e4,3))
rl=sum(b['stale'] for b in L)/sum(b['comment_bytes'] for b in L); rn=sum(b['stale'] for b in N)/sum(b['comment_bytes'] for b in N)
print('per-byte rate ratio',round(rl/rn,2))
print('block-level RR',round((sum(b['stale'] for b in L)/len(L))/(sum(b['stale'] for b in N)/len(N)),2))
print('mean bytes ratio',round((sum(b['comment_bytes'] for b in L)/len(L))/(sum(b['comment_bytes'] for b in N)/len(N)),2))
# Exact conditional test for rate ratio on bytes: k_L ~ Binomial(k, p=bytes_L/(bytes_L+bytes_N)), two-sided by summing small tails
kL=sum(b['stale'] for b in L); kN=sum(b['stale'] for b in N); n=kL+kN
bL=sum(b['comment_bytes'] for b in L); bN=sum(b['comment_bytes'] for b in N); p=bL/(bL+bN)
pm=lambda x: math.comb(n,x)*p**x*(1-p)**(n-x)
obs=pm(kL); pv=sum(pm(x) for x in range(n+1) if pm(x)<=obs*(1+1e-9))
print('exact binomial on bytes: p_bytes_share',round(p,3),'kL',kL,'n',n,'two-sided p',round(pv,4))
# weights: population dead claims by stratum
import collections
pop=defaultdict(lambda:[0,0,0,0])
for b in live:
    k=('>=1200' if b['comment_bytes']>=1200 else '<1200')
    pop[k][0]+=b['stale']*b['stratum_blocks']/ALLOC[(b['stratum_area'],b['band'])]
    pop[k][1]+=b['stale']*b['stratum_bytes']/ALLOC[(b['stratum_area'],b['band'])] 
print('population-weighted dead blocks',{k:round(v[0]) for k,v in pop.items()})
