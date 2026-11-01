# Stratum-weighted dead blocks and bytes for the two sizes, with a within-stratum bootstrap for the per-byte ratio.
import json,csv,random,sys
from collections import defaultdict
S='/private/tmp/claude-501/-Users-max-ws-chesso-build/8e195c86-e693-45ac-b65e-6be5437b5a74/scratchpad/regen/blocks.jsonl'
ALLOC={('src','xs'):6,('src','s'):16,('src','m'):18,('src','l'):10,('src','xl'):5,('tests','xs'):5,('tests','s'):16,('tests','m'):14,('tests','l'):8,('tests','xl'):3,('tools','xs'):4,('tools','s'):10,('tools','m'):8,('tools','l'):4,('tools','xl'):2,('adocs_data','xs'):3,('adocs_data','s'):5,('adocs_data','m'):5,('adocs_data','l'):4,('adocs_data','xl'):6,('root+cmake','xs'):1,('root+cmake','s'):2,('root+cmake','m'):4,('root+cmake','l'):3,('root+cmake','xl'):1}
bands=[(0,120,'xs'),(120,400,'s'),(400,1200,'m'),(1200,3000,'l'),(3000,10**9,'xl')]
def band(b):
    for lo,hi,n in bands:
        if lo<=b<hi: return n
B=[json.loads(l) for l in open(S)]
st=defaultdict(list)
for b in B:
    a='root+cmake' if b['area'] in('cmake','root') else b['area']
    b['sa']=a;b['band']=band(b['comment_bytes']);st[(a,b['band'])].append(b)
rng=random.Random(20261031);sample=[]
for k in sorted(ALLOC):
    pop=sorted(st[k],key=lambda b:(b['file'],b['start_line']))
    ch=rng.sample(pop,min(ALLOC[k],len(pop)))
    for c in ch: c['k']=k
    sample+=ch
sample.sort(key=lambda b:(b['sa'],b['file'],b['start_line']))
for i,b in enumerate(sample): b['id']=f"B{i+1:03d}"
cl={r['id']:r for r in csv.DictReader(open('/Users/max/ws/chesso/adocs/data/comment_study/classified.tsv'),delimiter='\t')}
for b in sample:b['stale']=int(cl[b['id']]['stale'])
live=[b for b in sample if b['sa']!='adocs_data']
strata=defaultdict(list)
for b in live:strata[b['k']].append(b)
popN={k:len(v) for k,v in st.items()};popB={k:sum(x['comment_bytes'] for x in v) for k,v in st.items()}
def long_band(k):return k[1] in('l','xl')
def est(sdict):
    dl=dn=0
    for k,v in sdict.items():
        rate=sum(x['stale'] for x in v)/len(v)
        if long_band(k):dl+=rate*popN[k]
        else:dn+=rate*popN[k]
    return dl,dn
dl,dn=est(strata)
bl=sum(popB[k] for k in strata if long_band(k));bn=sum(popB[k] for k in strata if not long_band(k))
print('dead blocks est, >=1200:',round(dl,1),' <1200:',round(dn,1),' total',round(dl+dn,1),'share long',round(dl/(dl+dn),3))
print('bytes long',bl,'short',bn,'share long',round(bl/(bl+bn),3))
print('per-10KB long',round(dl/bl*1e4,3),'short',round(dn/bn*1e4,3),'ratio',round((dl/bl)/(dn/bn),2))
r=random.Random(1);R=[];sh=[]
for _ in range(4000):
    d={k:[r.choice(v) for _ in v] for k,v in strata.items()}
    a,b=est(d)
    sh.append(a/(a+b) if a+b else 0)
    if b>0:R.append((a/bl)/(b/bn))
    else:R.append(float('inf'))
R.sort();sh.sort()
print('ratio 90% CI',round(R[200],2),R[3800] if R[3800]==float('inf') else round(R[3800],2),'share of draws with no short dead',sum(1 for x in R if x==float('inf'))/4000)
print('share-long 90% CI',round(sh[200],3),round(sh[3800],3))
