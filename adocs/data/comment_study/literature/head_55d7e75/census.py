#!/usr/bin/env python3
# Block-size census at HEAD with the study's own lexer, read-only over git blobs.
import sys, subprocess, json, collections
sys.path.insert(0, '/Users/max/ws/chesso/adocs/data/comment_study')
import count_comments as cc
from comment_lexer import language_of, lex, line_starts
REPO='/Users/max/ws/chesso'
ref=sys.argv[1] if len(sys.argv)>1 else 'HEAD'
files=subprocess.run(['git','-C',REPO,'ls-tree','-r','--name-only',ref],capture_output=True,text=True,check=True).stdout.split('\n')
rows=[]; nfiles=0; total=0; lines_total=0
per_area=collections.Counter()
for f in files:
    if not f: continue
    if f.startswith(cc.EXCLUDE_PREFIX): continue
    a=cc.area_of(f)
    if a is None or a=='adocs_data': continue
    try: lang=language_of(f)
    except Exception: lang=None
    if not lang: continue
    b=subprocess.run(['git','-C',REPO,'show',f'{ref}:{f}'],capture_output=True).stdout
    try: text=b.decode('utf-8')
    except UnicodeDecodeError: continue
    res=lex(f,text)
    comments=res[0]
    bl=cc.blockify(f,text,comments)
    nfiles+=1
    for x in bl:
        rows.append((f,a,x))
    per_area[a]+=sum(c.end_off-c.start_off for c in comments)
print('files',nfiles,'by area bytes',dict(per_area),'total',sum(per_area.values()))
print('sample block repr',rows[0][2] if rows else None)
json.dump([(f,a,x if isinstance(x,dict) else x.__dict__) for f,a,x in rows],open(sys.argv[2],'w'))
