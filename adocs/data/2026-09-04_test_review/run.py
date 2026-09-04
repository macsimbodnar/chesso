#!/usr/bin/env python3
"""Apply each mutant to the worktree, build, run the fast suite and the
node-count bench, record what caught it, revert. Results in results.tsv."""
import os, re, subprocess, sys, time, importlib.util

HERE = os.path.dirname(os.path.abspath(__file__))
WT = os.path.join(os.path.dirname(HERE), "mut")
BUILD = os.path.join(WT, "build")
ENGINE = os.path.join(BUILD, "src", "chesso")
BENCH = os.path.join(WT, "tools", "search_bench.py")
LOGS = os.path.join(HERE, "logs"); os.makedirs(LOGS, exist_ok=True)
RESULTS = os.path.join(HERE, "results.tsv")

spec = importlib.util.spec_from_file_location("mutants", os.path.join(HERE, "mutants.py"))
mod = importlib.util.module_from_spec(spec); spec.loader.exec_module(mod)
MUTANTS = mod.M
only = set(sys.argv[1:])

def sh(cmd, log=None, timeout=None):
    p = subprocess.run(cmd, cwd=WT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=timeout)
    if log:
        with open(log, "w") as f: f.write(p.stdout)
    return p.returncode, p.stdout

def revert():
    sh(["git", "checkout", "--", "src"])

def apply(mut):
    path = os.path.join(WT, mut["file"])
    text = open(path).read()
    for old, new in mut["pairs"]:
        n = text.count(old)
        if n != 1:
            return f"anchor x{n}"
        text = text.replace(old, new)
    open(path, "w").write(text)
    return None

def bench():
    rc, out = sh(["python3", BENCH, ENGINE, "9"], timeout=600)
    rows = re.findall(r"^\s+(\w+)\s+[\d.]+s\s+(\d+) nodes\s+\d+ knps\s+best (\S+)", out, re.M)
    return {name: (nodes, best) for name, nodes, best in rows}

def ctest(log):
    rc, out = sh(["ctest", "--test-dir", BUILD, "-L", "fast", "--output-on-failure"], log=log, timeout=3600)
    m = re.search(r"(\d+)% tests passed, (\d+) tests failed out of (\d+)", out)
    failed = re.findall(r"^\s*\d+ - (\S+) \((Failed|Timeout|.*?)\)\s*$", out, re.M)
    failed = [f"{name}({how})" for name, how in failed]
    return rc, (m.group(2) if m else "?"), (m.group(3) if m else "?"), failed

def main():
    revert()
    base_build_rc, _ = sh(["cmake", "--build", BUILD, "-j8"], log=os.path.join(LOGS, "baseline_build.log"))
    if base_build_rc != 0: sys.exit("baseline build failed")
    base = bench()
    rc, nf, nt, failed = ctest(os.path.join(LOGS, "baseline_ctest.log"))
    with open(RESULTS, "a" if only else "w") as f:
        if not only: f.write("id\tclass\tcompiled\tctest_failed\tctest_total\tfailed_tests\tnodes_changed\tbestmove_changed\tbench\tbuild_s\tctest_s\tnote\n")
        if not only: f.write(f"BASELINE\t-\tyes\t{nf}\t{nt}\t{' '.join(failed)}\t-\t-\t{base}\t-\t-\tunmutated worktree\n")
    print("baseline", nf, nt, failed, base, flush=True)
    for mut in MUTANTS:
        if only and mut["id"] not in only: continue
        revert()
        err = apply(mut)
        if err:
            row = [mut["id"], mut["klass"], "n/a", "-", "-", "", "-", "-", "", "-", "-", f"ANCHOR {err}: {mut['note']}"]
            print(row, flush=True)
            open(RESULTS, "a").write("\t".join(map(str, row)) + "\n"); continue
        t0 = time.time()
        brc, _ = sh(["cmake", "--build", BUILD, "-j8"], log=os.path.join(LOGS, mut["id"] + "_build.log"))
        build_s = round(time.time() - t0, 1)
        if brc != 0:
            row = [mut["id"], mut["klass"], "no", "-", "-", "", "-", "-", "", build_s, "-", mut["note"]]
        else:
            b = bench()
            nodes_changed = any(b.get(k, ("?", "?"))[0] != v[0] for k, v in base.items())
            best_changed = any(b.get(k, ("?", "?"))[1] != v[1] for k, v in base.items())
            t1 = time.time()
            rc, nf, nt, failed = ctest(os.path.join(LOGS, mut["id"] + "_ctest.log"))
            ctest_s = round(time.time() - t1, 1)
            row = [mut["id"], mut["klass"], "yes", nf, nt, " ".join(failed), "yes" if nodes_changed else "no",
                   "yes" if best_changed else "no", b, build_s, ctest_s, mut["note"]]
        print(row, flush=True)
        open(RESULTS, "a").write("\t".join(map(str, row)) + "\n")
    revert()
    sh(["cmake", "--build", BUILD, "-j8"])
    print("MUTATION-RUN-DONE", flush=True)

if __name__ == "__main__":
    main()
