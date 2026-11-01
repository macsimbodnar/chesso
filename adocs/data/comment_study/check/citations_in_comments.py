# What the checker's own recogniser sees in the pending step files, and which phrases rest on comment text.
import sys,os,re
os.chdir('/home/max/ws/chesso')
sys.path.insert(0,'tools')
import plan_prose_check as p
from pathlib import Path
adocs=Path('adocs')
tracked,byname=p._tracked()
pending=p.pending_step_files(adocs)
c={'symbol':0,'phrase':0,'line':0,'bare':0}
only=[]
for f in pending:
    text=open(f).read()
    for first,para in p.paragraphs(text):
        for line,sp,res,kind,val in p.refs_in(para,first,tracked,byname):
            c[kind]+=1
            if kind=='phrase' and res and p.gated(res):
                code=p.code_of(res) or ''
                needle=' '.join(val.split()).lower()
                if needle not in ' '.join(code.split()).lower() and ' '.join(val.split()) not in p.titles_of(res):
                    only.append((str(f),res,val[:70]))
print(c); print('phrase citations resting on comment text:',len(only))
for x in only: print('  ',x)
