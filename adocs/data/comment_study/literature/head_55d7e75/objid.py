# Release objects of the engine with every comment stripped, against HEAD's sources. Scratch only.
import sys, subprocess, os, shutil, hashlib, time, concurrent.futures as cf
sys.path.insert(0,'/Users/max/ws/chesso/adocs/data/comment_study')
from comment_lexer import language_of, strip_comments
R='/Users/max/ws/chesso'; W=os.path.abspath('objid'); shutil.rmtree(W,ignore_errors=True)
files=subprocess.run(['git','-C',R,'ls-tree','-r','--name-only','HEAD','src/'],capture_output=True,text=True).stdout.split()
for arm in 'AB':
    os.makedirs(f'{W}/{arm}/src',exist_ok=True); os.makedirs(f'{W}/{arm}/gen',exist_ok=True)
    shutil.copy(f'{R}/build/generated/chesso_build_info.hpp',f'{W}/{arm}/gen/')
    for f in files:
        b=subprocess.run(['git','-C',R,'show',f'HEAD:{f}'],capture_output=True).stdout
        p=f'{W}/{arm}/{f}'
        if arm=='B' and language_of(f)=='cpp' and not f.endswith('.S'):
            b=strip_comments(f,b.decode()).encode()
        open(p,'wb').write(b)
tus=[f for f in files if f.endswith('.cpp')]
def cc(arm,tu,flags,tag):
    o=f'{W}/{arm}/{os.path.basename(tu)}.{tag}.o'
    p=subprocess.run(['/usr/bin/c++','-Isrc','-Igen',*flags,'-std=gnu++20','-arch','arm64','-march=native','-o',o,'-c',tu],cwd=f'{W}/{arm}',capture_output=True,text=True)
    if p.returncode: return tu,arm,'ERR '+p.stderr[:300]
    return tu,arm,hashlib.sha256(open(o,'rb').read()).hexdigest()[:16]
t0=time.time()
for tag,flags in (('rel',['-O3','-DNDEBUG']),('dbg',['-O1','-UNDEBUG'])):
    with cf.ThreadPoolExecutor(8) as ex:
        res=list(ex.map(lambda a:cc(*a,flags,tag),[(arm,tu) for tu in tus for arm in 'AB']))
    d={}
    for tu,arm,h in res: d.setdefault(tu,{})[arm]=h
    same=[t for t,v in d.items() if v['A']==v['B']]; diff=[t for t,v in d.items() if v['A']!=v['B']]
    print(tag,'same',len(same),'diff',len(diff),diff); 
    for t,v in d.items():
        if 'ERR' in v['A'] or 'ERR' in v['B']: print(t,v)
print('secs',round(time.time()-t0))
