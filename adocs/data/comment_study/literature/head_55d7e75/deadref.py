# Mechanical dead-reference probes over live comment text at HEAD. Noisy by design: counts of candidates, not of falsehoods.
import sys, subprocess, re, os, json, collections
sys.path.insert(0,'/Users/max/ws/chesso/adocs/data/comment_study')
import count_comments as cc
from comment_lexer import language_of, lex
R='/Users/max/ws/chesso'
def sh(*a): return subprocess.run(['git','-C',R,*a],capture_output=True).stdout
tree=set(sh('ls-tree','-r','--name-only','HEAD').decode().split('\n'))
done={os.path.basename(p)[:4] for p in os.listdir(f'{R}/adocs/plan_done') if re.match(r'S\d{3}',p)}
todo={os.path.basename(p)[:4] for p in os.listdir(f'{R}/adocs/plan_todo') if re.match(r'S\d{3}',p)}
cur={os.path.basename(p)[:4] for p in os.listdir(f'{R}/adocs/plan_current') if re.match(r'S\d{3}',p)}
dec=set(re.findall(r'^## (DEC-\d+)',open(f'{R}/adocs/decisions.md',errors='replace').read(),re.M))
inv=set(re.findall(r'INV-\d+',open(f'{R}/adocs/specs.md',errors='replace').read()))
FILELINE=re.compile(r'\b([\w./-]+\.(?:cpp|hpp|py|sh|md|txt|cmake)):(\d+)(?:-\d+)?\b')
PAR=re.compile(r'AGENTS\.md par\.?\s*\d+|surface_guard')
FWD=re.compile(r"\b(S\d{3})(?:'s)?\s+(?:owns?|owes?|will|is pending|is open|takes this|moves this|holds this|has yet|remains|still owes)\b|\b(?:until|once|after|when|before|waits? (?:for|on))\s+(S\d{3})\s+(?:lands?|lands|lands,|closes?|lands\.|is done|lands;|ships?)\b|\b(?:an? H[01] (?:on|for)|on an H[01] (?:on|for)|pending)\s+(S\d{3})\b")
SID=re.compile(r'\bS(\d{3})\b');DID=re.compile(r'\bDEC-(\d{3})\b');IID=re.compile(r'\bINV-\d+\b')
PATH=re.compile(r'(?<![\w/.-])((?:src|tests|tools|cmake|adocs)/[\w./-]+\.\w+)')
c=collections.Counter();ex=collections.defaultdict(list);totalids=collections.Counter()
for f in sorted(tree):
    if not f or f.startswith(cc.EXCLUDE_PREFIX): continue
    a=cc.area_of(f)
    if a in (None,'adocs_data') or not language_of(f): continue
    b=sh('show',f'HEAD:{f}')
    try:t=b.decode()
    except: continue
    for m in lex(f,t)[0]:
        s=t[m.start_off:m.end_off]
        ln=m.start_line
        for x in FILELINE.finditer(s): c['fileline']+=1; ex['fileline'].append((f,ln,x.group(0)))
        for x in PAR.finditer(s): c['par']+=1; ex['par'].append((f,ln,x.group(0)))
        for x in FWD.finditer(s):
            sid=x.group(1) or x.group(2) or x.group(3)
            if sid in done: c['fwd_closed']+=1; ex['fwd_closed'].append((f,ln,x.group(0)))
            else: c['fwd_open']+=1
        for x in SID.finditer(s):
            sid='S'+x.group(1); totalids['S']+=1
            if sid not in done|todo|cur: c['unres_S']+=1; ex['unres_S'].append((f,ln,sid))
        for x in DID.finditer(s):
            did='DEC-'+x.group(1); totalids['DEC']+=1
            if did not in dec: c['unres_DEC']+=1; ex['unres_DEC'].append((f,ln,did))
        for x in IID.finditer(s):
            totalids['INV']+=1
            if x.group(0) not in inv: c['unres_INV']+=1; ex['unres_INV'].append((f,ln,x.group(0)))
        for x in PATH.finditer(s):
            p=x.group(1).rstrip('.,;:')
            if '*' in p or '<' in p: continue
            totalids['path']+=1
            if p not in tree and not any(g.startswith(p) for g in tree): c['gone_path']+=1; ex['gone_path'].append((f,ln,p))
print(dict(c));print(dict(totalids))
for k in ex:
    print(k,len(ex[k]),ex[k][:6])
print('distinct gone paths',len({x[2] for x in ex['gone_path']}))
print('steps done',len(done),'todo',len(todo),'cur',len(cur),'decs',len(dec))
