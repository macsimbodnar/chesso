import json, collections, re, subprocess, sys
rows=json.load(open('blocks_head.json'))
blocks=[(f,a,b) for f,a,b in rows]
tot=sum(b['comment_bytes'] for _,_,b in blocks)
print('blocks',len(blocks),'comment_bytes',tot)
def lines(b): return b['end_line']-b['start_line']+1
bands=[(0,200),(200,500),(500,1200),(1200,3000),(3000,7000),(7000,10**9)]
for lo,hi in bands:
    s=[b for _,_,b in blocks if lo<=b['comment_bytes']<hi]
    print(f'{lo:>6}-{hi:<10} blocks {len(s):5d} bytes {sum(b["comment_bytes"] for b in s):8d} share {100*sum(b["comment_bytes"] for b in s)/tot:5.1f}% lines {sum(lines(b) for b in s):6d}')
ge=[(f,a,b) for f,a,b in blocks if b['comment_bytes']>=1200]
print('>=1200B',len(ge),sum(b['comment_bytes'] for _,_,b in ge),'files',len({f for f,_,_ in ge}))
g15=[(f,a,b) for f,a,b in blocks if lines(b)>15]
print('>15 lines',len(g15),sum(b['comment_bytes'] for _,_,b in g15),'files',len({f for f,_,_ in g15}))
both=[x for x in ge if lines(x[2])>15]
print('both',len(both),sum(b['comment_bytes'] for _,_,b in both))
for a in ['src','tests','tools','cmake','root']:
    s=[b for f,ar,b in blocks if ar==a]
    l=[b for b in s if b['comment_bytes']>=1200]
    print(a,'blocks',len(s),'bytes',sum(b['comment_bytes'] for b in s),'>=1200:',len(l),sum(b['comment_bytes'] for b in l))
print('total comment lines', sum(lines(b) for _,_,b in blocks))
med=sorted(b['comment_bytes'] for _,_,b in blocks)[len(blocks)//2]
print('median block',med)
perfile=collections.Counter()
for f,a,b in blocks: perfile[f]+=b['comment_bytes']
print(perfile.most_common(8))
print('files over 75KB comment:',[ (f,v) for f,v in perfile.items() if v>75000])
# batches at 75 KB: greedy lower bound
print('batches at 75KB (all):',round(tot/75000,1),' at 97KB:',round(tot/97000,1))
