#!/usr/bin/env python3
"""What the pilot removed: exact facts in removed comment text, looked up in the record."""
import re,subprocess,sys
sys.path.insert(0,'..')
from comment_lexer import lex
REPO='/home/max/ws/chesso';C='5954c12'
def g(*a): return subprocess.run(['git','-C',REPO,*a],capture_output=True).stdout.decode('utf-8','replace')
tree=g('ls-tree','-r','--name-only',C).split('\n')
docs={}
for p in tree:
    if re.match(r'adocs/(plan_done|plan_todo|plan_current|audit)/',p) or p in('adocs/decisions.md','adocs/specs.md','DEV_MANUAL.md','MANUAL.md'):
        docs[p]=g('show',f'{C}:{p}')
    elif p.startswith('adocs/data/') and re.search(r'\.(txt|md|tsv|sh|py)$',p):
        docs[p]=g('show',f'{C}:{p}')[:400000]
log=g('log','--format=%B',C)
docs['git log']=log
FACT=re.compile(r"`([^`\n]{3,50})`|(?<![\w.])(\d{4,}(?:\.\d+)?)\b|(?<![\w.])(\d+\.\d+)\b|\b(\d+ of \d+|\d+ -> \d+|\d+ %)")
step={}
for p in docs:
    m=re.match(r'adocs/plan_\w+/(S\d+)_',p)
    if m: step[m.group(1)]=p
for path,out in (('src/search.cpp','search.cpp'),('tests/test_mate_carry.cpp','test_mate_carry.cpp')):
    before=g('show',f'{C}:{path}')
    after=open(out).read()
    def comment_text(t,p):
        cs,_=lex(p,t)
        return ' '.join(re.sub(r'(?m)^\s*//+\s?','',t[c.start_off:c.end_off]) for c in cs)
    a=comment_text(after,path)
    b=comment_text(before,path)
    if path=='src/search.cpp':
        # only the pilot's section
        cs,_=lex(path,before)
        b=' '.join(re.sub(r'(?m)^\s*//+\s?','',before[c.start_off:c.end_off]) for c in cs if 1781<=c.start_line<=2089)
    flat=lambda s:re.sub(r'\s+',' ',s)
    a=flat(a);b=flat(b)
    facts=[]
    for m in FACT.finditer(b):
        f=m.group(1) or m.group(2) or m.group(3) or m.group(4)
        if f not in a: facts.append(f)
    facts=sorted(set(facts))
    sids=set(re.findall(r'\bS\d{3}\b',b)); dids=set(re.findall(r'\bDEC-\d{3}\b',b))
    cited=[step[s] for s in sids if s in step]
    cited_text=' '.join(flat(docs[c]) for c in cited)
    dec=docs['adocs/decisions.md']
    for d in dids:
        m=re.search(r'(?ms)^## '+d+r'\b.*?(?=^## DEC-|\Z)',dec)
        if m: cited_text+=' '+flat(m.group(0))
    every=' '.join(flat(t) for t in docs.values())
    byid=[f for f in facts if f in cited_text]
    anyw=[f for f in facts if f not in cited_text and f in every]
    none=[f for f in facts if f not in cited_text and f not in every]
    print(path,'removed exact facts',len(facts),'in a cited doc',len(byid),'elsewhere',len(anyw),'nowhere',len(none))
    print('  nowhere:',none[:40])
    ids_before=len(sids)+len(dids)
    ids_after=len(set(re.findall(r'\bS\d{3}\b',a))|set(re.findall(r'\bDEC-\d{3}\b',a)))
    print('  ids before',ids_before,'after',ids_after)
