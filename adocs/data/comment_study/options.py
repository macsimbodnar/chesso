#!/usr/bin/env python3
"""Question 9: the options' arithmetic, from measured inputs, in one place."""
import csv, json, math
rows=[json.loads(l) for l in open('transcript_rows.jsonl') if not json.loads(l)['excluded']]
reg=json.load(open('token_regression.json'))
SCALE=reg['per_comment_byte']*4   # real tokens per comment byte against chars/4
tot_w=sum(r['totals'].get('weighted_total',0) for r in rows)
tot_r=sum(r['totals'].get('raw_context',0) for r in rows)
def fam(k):
    k=k.split(':',1)[-1] if k.startswith(('Read:','Grep:','Glob:')) else k
    return k.replace('Bash:read/','').replace('Bash:','')
w={};raw={}
for r in rows:
    for k,v in r['w_comment'].items(): w[fam(k)]=w.get(fam(k),0)+v
    for k,v in r['raw_comment'].items(): raw[fam(k)]=raw.get(fam(k),0)+v
print('scale real/chars4',round(SCALE,3))
print('corpus weighted units',int(tot_w),'raw',int(tot_r))
print('comment weighted share chars/4 %.2f%%, real %.2f%%'%(100*sum(w.values())/tot_w,100*SCALE*sum(w.values())/tot_w))
print('comment raw share chars/4 %.2f%%, real %.2f%%'%(100*sum(raw.values())/tot_r,100*SCALE*sum(raw.values())/tot_r))
live=['src','tests','tools','root_scripts']
wl=sum(w.get(k,0) for k in live); rl=sum(raw.get(k,0) for k in live)
print('live code only (no adocs/data, no diffs): weighted chars/4 %.2f%% real %.2f%%; raw chars/4 %.2f%% real %.2f%%'%(100*wl/tot_w,100*SCALE*wl/tot_w,100*rl/tot_r,100*SCALE*rl/tot_r))
modes=json.load(open('read_modes.json'))
c=lambda m:modes.get(m+'|comment',0)
cw=c('Read whole')+c('Bash cat/other')+c('Bash grep'); cwin=c('Read window')+c('Bash sed window')
total=cw+cwin
print('comment bytes read: whole+grep %d (%.0f%%), windows %d (%.0f%%)'%(cw,100*cw/total,cwin,100*cwin/total))
lo=cw/total; mid=lo+0.5*(1-lo); hi=1.0
print('pass-through factors low %.2f mid %.2f high 1.00'%(lo,mid))
# Typical step: subagents attributed to a step by id
big=[r for r in rows if r['kind']!='main' and r['requests']>=3]
by={}
for r in big:
    ids=r['desc_steps'] or r['brief_steps'][:1]
    if ids: by.setdefault(ids[0],[]).append(r)
S=[]
for s,rs in by.items():
    wt=sum(r['totals']['weighted_total'] for r in rs); rt=sum(r['totals']['raw_context'] for r in rs)
    wc_all=sum(sum(r['w_comment'].values()) for r in rs); rc_all=sum(sum(r['raw_comment'].values()) for r in rs)
    wc=sum(v for r in rs for k,v in r['w_comment'].items() if fam(k) in live)
    rc=sum(v for r in rs for k,v in r['raw_comment'].items() if fam(k) in live)
    tok=sum(v for r in rs for k,v in r['item_comment_tokens'].items() if fam(k) in live)
    S.append(dict(s=s,wt=wt,rt=rt,wc=wc,rc=rc,tok=tok,wc_all=wc_all,rc_all=rc_all))
med=lambda a:sorted(a)[len(a)//2]
qt=lambda a,p:sorted(a)[int(p*len(a))]
print('steps',len(S),'median weighted',int(med([x['wt'] for x in S])),'mean',int(sum(x['wt'] for x in S)/len(S)),'median raw',int(med([x['rt'] for x in S])),'mean raw',int(sum(x['rt'] for x in S)/len(S)))
shw=[SCALE*x['wc']/x['wt'] for x in S if x['wt']]; shr=[SCALE*x['rc']/x['rt'] for x in S if x['rt']]
print('live-code comment share per step, real tokens: weighted med %.1f%% IQR %.1f-%.1f; raw med %.1f%% IQR %.1f-%.1f'%(100*med(shw),100*qt(shw,.25),100*qt(shw,.75),100*med(shr),100*qt(shr,.25),100*qt(shr,.75)))
print('comment tokens entering context per step (live code, chars/4 %d, real %d)'%(med([x['tok'] for x in S]),SCALE*med([x['tok'] for x in S])))
aggw=SCALE*sum(x['wc'] for x in S)/sum(x['wt'] for x in S); aggr=SCALE*sum(x['rc'] for x in S)/sum(x['rt'] for x in S)
print('aggregate over steps: weighted %.2f%%, raw %.2f%%'%(100*aggw,100*aggr))
main=[r for r in rows if r['kind']=='main']
mw=sum(r['totals'].get('weighted_total',0) for r in main); mr=sum(r['totals'].get('raw_context',0) for r in main)
mc=sum(v for r in main for k,v in r['w_comment'].items() if fam(k) in live)
print('coordinator weighted per attributed step %d, live comment share real %.2f%%'%(mw/len(S),100*SCALE*mc/mw))
medw=med([x['wt'] for x in S]); meanw=sum(x['wt'] for x in S)/len(S); medr=med([x['rt'] for x in S])
sh=med(shw); shR=med(shr)
print()
print('Per step savings, median step, subagents only; share of live-code comments x cut x pass-through')
for cut in (0.3,0.6):
    for name,pt in (('low',lo),('mid',mid),('high',hi)):
        print(f'  cut {int(cut*100)}% pass {name}: weighted {int(sh*cut*pt*medw)} units ({100*sh*cut*pt:.2f}%), raw {int(shR*cut*pt*medr)} tokens ({100*shR*cut*pt:.2f}%)')
allin=(sh*medw+0)/ (medw+mw/len(S))
print('all-in with the coordinator: comment share %.2f%%'%(100*allin))
files={r['file']:r for r in csv.DictReader(open('files.tsv'),delimiter='\t')}
read=json.load(open('read_files.json'))
rc={}
for k,v in read.items():
    p,wh=k.split('|')
    if wh=='comment': rc[p]=v
trc=sum(rc.values())
live_cb=sum(int(r['comment_bytes']) for r in files.values() if r['area']!='adocs_data')
all_cb=sum(int(r['comment_bytes']) for r in files.values())
print('live comment bytes',live_cb,'all',all_cb)
cand=[p for p in rc if p in files and files[p]['area']!='adocs_data']
cand.sort(key=lambda p:-rc[p]/max(1,int(files[p]['comment_bytes'])))
scored=sorted(cand,key=lambda p:-rc[p])[:40]
# pick by traffic per comment byte among files with a real share of traffic
pick=[p for p in sorted(cand,key=lambda p:-rc[p]/int(files[p]['comment_bytes'])) if rc[p]/trc>=0.01]
acc_t=acc_b=0
print('files with >=1% of comment reads, by traffic per comment byte:')
for p in pick:
    acc_t+=rc[p];acc_b+=int(files[p]['comment_bytes'])
    print(f"  {p:38s} cb {int(files[p]['comment_bytes']):7d} read {rc[p]:8d} share {100*rc[p]/trc:4.1f}%  x{rc[p]/int(files[p]['comment_bytes']):5.1f} cum_bytes {acc_b:7d} cum_share {100*acc_t/trc:5.1f}%")
json.dump(dict(pick=pick,scale=SCALE,lo=lo,mid=mid,medw=medw,meanw=meanw,sh=sh,shR=shR,medr=medr,live_cb=live_cb,pick_bytes=acc_b,pick_share=acc_t/trc,totw=tot_w,wl=wl),open('options.json','w'))
# cost model
print()
UNITS_LOW,UNITS_HIGH=3.5,10
for name,bytes_,cut,share in (('B all live code',live_cb,0.55,1.0),('B-narrow',acc_b,0.55,acc_t/ (trc*sum(1 for _ in [0]) ) ),('C all live code',live_cb,0.75,1.0)):
    # share is of all comment reads; live code total share:
    live_share=sum(rc[p] for p in rc if p in files and files[p]['area']!='adocs_data')/trc
    s=share if name.startswith('B-') else live_share
    frac=SCALE*sum(w.values())/tot_w*s*cut
    print(f'{name}: {bytes_} comment bytes, cost {bytes_*UNITS_LOW/1e6:.1f}-{bytes_*UNITS_HIGH/1e6:.1f}M units; saving high {100*frac:.2f}% mid {100*frac*mid:.2f}% low {100*frac*lo:.2f}% of weighted; per mean step {int(frac*mid*meanw)} mid; payback mean steps {bytes_*UNITS_LOW/(frac*hi*meanw):.0f}-{bytes_*UNITS_HIGH/(frac*lo*meanw):.0f}, mid {bytes_*(UNITS_LOW+UNITS_HIGH)/2/(frac*mid*meanw):.0f}; days at corpus rate {bytes_*(UNITS_LOW+UNITS_HIGH)/2/(frac*mid*tot_w/79):.0f}')
