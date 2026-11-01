# How often agents went to history at all: counts only, by command shape.
import glob,json,re
from collections import Counter
R='/home/max/.claude/projects/-home-max-ws-chesso'
c=Counter();res=Counter();ids={}
for p in glob.glob(R+'/*.jsonl')+glob.glob(R+'/*/subagents/*.jsonl'):
    with open(p,errors='replace') as fh:
        for line in fh:
            try:d=json.loads(line)
            except ValueError: continue
            if d.get('type')=='assistant':
                for x in (d.get('message') or {}).get('content') or []:
                    if isinstance(x,dict) and x.get('type')=='tool_use' and x.get('name')=='Bash':
                        cmd=(x.get('input') or {}).get('command','')
                        c['bash']+=1
                        kind=None
                        if re.search(r'git( -C \S+)? log[^|;&]*( -S| -G| -L)',cmd): kind='git log -S/-G/-L'
                        elif re.search(r'git( -C \S+)? blame',cmd): kind='git blame'
                        elif re.search(r'git( -C \S+)? log[^|;&]*--grep',cmd): kind='git log --grep'
                        elif re.search(r'git( -C \S+)? log',cmd): kind='git log other'
                        elif re.search(r'git( -C \S+)? show',cmd): kind='git show'
                        if kind:
                            c[kind]+=1; ids[x.get('id')]=kind
            elif d.get('type')=='user':
                m=(d.get('message') or {}).get('content')
                if isinstance(m,list):
                    for x in m:
                        if isinstance(x,dict) and x.get('type')=='tool_result' and x.get('tool_use_id') in ids:
                            t=x.get('content')
                            n=len(t) if isinstance(t,str) else sum(len(y.get('text','')) for y in t if isinstance(y,dict))
                            res[ids[x['tool_use_id']]]+=n
print(dict(c)); print({k:(res[k],res[k]//max(1,c[k])) for k in res})
