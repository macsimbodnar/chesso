#!/usr/bin/env python3
"""Prints the census tables, computed and not retyped."""
import csv
from collections import defaultdict
rows=list(csv.DictReader(open('files.tsv'),delimiter='\t'))
keys=('bytes','lines','code_lines','comment_lines','blank_lines','comment_bytes','removable_bytes','blocks')
ag=defaultdict(lambda:defaultdict(int))
for r in rows:
    a=r['area']
    for g in (a,'TOTAL')+(('LIVE',) if a!='adocs_data' else ()):
        ag[g]['files']+=1
        for k in keys: ag[g][k]+=int(r[k])
names={'src':'`src/`','tests':'`tests/`','tools':'`tools/`','cmake':'`cmake/`','root':'root scripts and `CMakeLists.txt`','LIVE':'**live, all above**','adocs_data':'`adocs/data/` scripts','TOTAL':'**census**'}
L=['| area | files | code lines | comment lines | blocks | comment bytes | tokens, chars/4 | share of bytes | share of code+comment lines |','|---|---|---|---|---|---|---|---|---|']
for a in ('src','tests','tools','cmake','root','LIVE','adocs_data','TOTAL'):
    v=ag[a]
    L.append(f"| {names[a]} | {v['files']} | {v['code_lines']} | {v['comment_lines']} | {v['blocks']} | {v['comment_bytes']} | {v['comment_bytes']//4} | {100*v['removable_bytes']/v['bytes']:.1f} % | {100*v['comment_lines']/(v['comment_lines']+v['code_lines']):.1f} % |")
Q1='\n'.join(L)
rows.sort(key=lambda r:-int(r['comment_bytes']))
T=['| file | bytes | code lines | comment lines | comment bytes | tokens, chars/4 | share of bytes |','|---|---|---|---|---|---|---|']
for r in rows[:20]:
    T.append(f"| `{r['file']}` | {r['bytes']} | {r['code_lines']} | {r['comment_lines']} | {r['comment_bytes']} | {int(r['comment_bytes'])//4} | {100*int(r['removable_bytes'])/int(r['bytes']):.1f} % |")
Q2='\n'.join(T)
print(Q1); print(); print(Q2)
