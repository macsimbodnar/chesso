#!/usr/bin/env python3
"""A `{0, 5}` nElo SPRT's stopping bias, simulated at S125's pair variance.

Monte Carlo on pair scores with normal increments, sd 0.2593 from S125's pentanomial
[70, 269, 465, 326, 124], 1500 runs a row, seed 1, no engine and no game: the coordinator's
reading of 2026-10-31, an approximation and no verdict. Prints each true nElo's H1 rate, median
games and mean estimate at H1, then the truth consistent with stopping at 2000 to 3000 games on
a flat prior. Standard library only.
"""
import math, random, statistics as st
random.seed(1)
sd=0.2593; K=347.4; c=math.sqrt(2)*sd
mu1=5/K*c; LO=-2.94; HI=2.94; cap=20950
def run(t):
    d=t/K*c; n=0; s=0.0
    a=mu1/sd**2
    while n<cap:
        n+=1; s+=random.gauss(d,sd)
        llr=a*(s-n*mu1/2)
        if llr>=HI: return 1,n,(s/n)/c*K
        if llr<=LO: return 0,n,(s/n)/c*K
    return 2,n,(s/n)/c*K
print("true_nElo  P(H1)  median_games  mean_est_at_H1  bias")
for t in [5,7.5,10,15,20,25,30,40]:
    r=[run(t) for _ in range(1500)]
    h=[x for x in r if x[0]==1]
    g=st.median(x[1]*2 for x in h); m=st.mean(x[2] for x in h)
    print(f"{t:8.1f} {len(h)/len(r):6.2f} {g:12.0f} {m:15.1f} {m-t:6.1f}")
# of the H1 runs stopping at about S125's length, what truth is consistent, flat prior 0..60
post={}
for t in range(0,61,2):
    r=[run(t) for _ in range(1500)]
    post[t]=sum(1 for x in r if x[0]==1 and 1000<=x[1]<=1500)
tot=sum(post.values()); mean=sum(t*v for t,v in post.items())/tot
acc=0; q={}
for t in sorted(post):
    acc+=post[t]
    for p in (.05,.5,.95):
        if p not in q and acc/tot>=p: q[p]=t
print("stopping at 2000-3000 games, flat prior: mean",round(mean,1),"5/50/95%",q[.05],q[.5],q[.95])
