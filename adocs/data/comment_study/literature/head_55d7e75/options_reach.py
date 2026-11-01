# Traffic and open-step reach of each DEC-336 tier at HEAD, for the fix-or-leave comparison.
import json,re,glob,os,subprocess,collections
R='/Users/max/ws/chesso'
rows=json.load(open('blocks_head.json'))
read=json.load(open(R+'/adocs/data/comment_study/read_files.json'))
rc={k.split('|')[0]:v for k,v in read.items() if k.endswith('|comment')}
ID=re.compile(r'\bS\d{3}\b|\bDEC-\d{3}\b')
cache={}
def blob(p):
    if p not in cache: cache[p]=subprocess.run(['git','-C',R,'show',f'HEAD:{p}'],capture_output=True).stdout.decode()
    return cache[p]
filetot=collections.Counter()
for f,a,b in rows: filetot[f]+=b['comment_bytes']
live_read=sum(v for k,v in rc.items() if k in filetot)
tiers=collections.defaultdict(list)
for f,a,b in rows:
    t=blob(f)[b['start_off']:b['end_off']]; ids=len(set(ID.findall(t))); n=b['comment_bytes']
    tier='T1' if n>=1200 and ids>=6 and ids/(n/1000)>=1.5 else ('T2' if n>=1200 else 'T3')
    tiers[tier].append((f,a,n))
# files named by open steps' touches
touch=collections.defaultdict(set)
for p in glob.glob(R+'/adocs/plan_todo/S*.md')+glob.glob(R+'/adocs/plan_current/S*.md'):
    s=os.path.basename(p)[:4]
    if s in('S311','S312','S313','S308','S314','S315'): continue
    m=re.search(r'^touches:(.*)$',open(p,encoding='utf8').read(),re.M)
    for w in re.findall(r'[\w./-]+\.(?:cpp|hpp|py|sh|txt|S)|\bsrc/|\btests/|\btools/',m.group(1) if m else ''):
        touch[w].add(s)
named={w for w in touch if not w.endswith('/')}
print('open steps touch files',len(named),'dirs',[w for w in touch if w.endswith('/')])
tot=sum(n for v in tiers.values() for _,_,n in v)
for t in('T1','T2','T3'):
    v=tiers[t]; b=sum(x[2] for x in v)
    trf=sum(rc.get(f,0)*n/filetot[f] for f,_,n in v)
    tb=sum(n for f,_,n in v if f in named)
    print(t,'blocks',len(v),'bytes',b,'%.1f%% of bytes'%(100*b/tot),'share of live comment reads %.1f%%'%(100*trf/live_read),'in files open steps name %.1f%% of tier bytes'%(100*tb/b),'files',len({f for f,_,_ in v}))
top=collections.Counter()
for s in touch.values():
    pass
