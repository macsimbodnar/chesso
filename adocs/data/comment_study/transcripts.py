#!/usr/bin/env python3
"""Question 5: where tokens go, by script over the session transcripts. Stdlib only.

Measured from `usage`, per API request, deduplicated by requestId: input, cache
write (5 m and 1 h), cache read, output, thinking. Tool result sizes are the bytes
the model saw. Tokens of a context item are bytes/4 (an estimate); the request
totals are the API's own numbers.

For each request the context is split by category, using what has arrived so far
in that transcript, reset at a compaction. An item's raw load is its tokens at each
request it sits in. Its weighted load uses the request's own cache behaviour: a new
item is a cache write; an old item is a cache read, or a write in the proportion of
the old context the request re-wrote (a miss after a long wait).

Weights, in input-token units, from the claude-api skill's price table: cache read
0.05 on claude-opus-5-5 and 0.1 otherwise, write 1.25 at 5 m and 2 at 1 h, output 5.

Writes transcript_rows.jsonl (one row per transcript) and transcript_summary.json.
No transcript text is written, only paths, counts and step ids.
"""
import glob
import json
import os
import re
import sys
from collections import Counter, defaultdict

ROOT = "/home/max/.claude/projects/-home-max-ws-chesso"
OUT = os.path.dirname(os.path.abspath(__file__))

STRIP = re.compile(r"^.*?/ws/chesso(?:-[A-Za-z0-9_]+)?/")
WT = re.compile(r"^(?:\.ref-builds/[^/]+/|\.tuning/.*?/tree/)")
ROOT_DOCS = {"AGENTS.md", "CLAUDE.md", "DEV_MANUAL.md", "MANUAL.md", "TOOLCHAIN.md", "README.md"}
ROOT_SCRIPTS = {"fastchess.sh", "rating.sh", "build_release.sh", "clang-format.sh", "test_uci.sh", "CMakeLists.txt"}


def category_of_path(p):
    if not p:
        return "unknown"
    p = p.strip("'\"")
    if "/ws/chesso" in p:
        p = STRIP.sub("", p)
    elif p.startswith("/"):
        if "/.claude/" in p:
            return "claude_home"
        if p.startswith("/tmp"):
            return "tmp"
        return "outside"
    p = WT.sub("", p.lstrip("./"))
    if p.startswith(".tuning/coord/") or p.startswith("tuning/coord/"):
        return "briefs" if "brief" in p else "tuning"
    if p.startswith(".tuning") or p.startswith("tuning/") or p.startswith(".ref-builds") or p.startswith("ref-builds"):
        return "tuning"
    for pre, name in (
        ("src/", "src"),
        ("tests/", "tests"),
        ("tools/", "tools"),
        ("cmake/", "root_scripts"),
        ("adocs/status.md", "adocs/status"),
        ("adocs/plan.md", "adocs/plan"),
        ("adocs/specs.md", "adocs/specs"),
        ("adocs/decisions.md", "adocs/decisions"),
        ("adocs/plan_done", "adocs/plan_done"),
        ("adocs/plan_todo", "adocs/plan_todo"),
        ("adocs/plan_current", "adocs/plan_current"),
        ("adocs/audit", "adocs/audit"),
    ):
        if p.startswith(pre):
            return name
    if p.startswith("adocs/data/"):
        return "adocs/data_script" if re.search(r"\.(py|sh|awk|cpp|hpp)$", p) else "adocs/data_other"
    if p.startswith("adocs/"):
        return "adocs/other"
    if p in ROOT_DOCS:
        return "root_docs"
    if p in ROOT_SCRIPTS:
        return "root_scripts"
    return "other_path"


CODE_CATS = {"src", "tests", "tools", "root_scripts", "adocs/data_script"}
PATHTOK = re.compile(
    r"(?:^|[\s'\"=<(])((?:/[\w./+-]*ws/chesso[\w./+-]*)|(?:\.{0,2}/)?(?:\.tuning|src|tests|tools|adocs|cmake)/[\w./+-]*|(?:AGENTS|CLAUDE|DEV_MANUAL|MANUAL|TOOLCHAIN)\.md|(?:fastchess|rating|build_release|clang-format)\.sh|CMakeLists\.txt)"
)
LANG_EXT = {
    "cpp": "cpp", "hpp": "cpp", "h": "cpp", "cc": "cpp", "S": "cpp",
    "py": "py", "sh": "hash", "cmake": "hash", "awk": "hash", "txt": "hash",
}


def lang_of(path):
    if not path:
        return None
    if path.endswith("CMakeLists.txt"):
        return "hash"
    m = re.search(r"\.([A-Za-z]+)$", path)
    if not m:
        return None
    if m.group(1) == "txt":
        return None
    return LANG_EXT.get(m.group(1))


PREFIX = re.compile(r"^\s*\d+[\t→]")
GREP_PREFIX = re.compile(r"^(?:[\w./+-]+[:-])?\d+[:-]")


def comment_bytes(text, lang, grep=False):
    """Line-level count of comment bytes in text a tool returned. Whole comment lines
    only; a partial window can start inside a block, so this is a floor."""
    if not lang or not text:
        return 0
    total = 0
    in_block = False
    in_doc = False
    for raw in text.split("\n"):
        line = PREFIX.sub("", raw, count=1)
        if grep:
            line = GREP_PREFIX.sub("", line, count=1)
        s = line.strip()
        n = len(raw) + 1
        if lang == "cpp":
            if in_block:
                total += n
                if "*/" in s:
                    in_block = False
                continue
            if s.startswith("//"):
                total += n
            elif s.startswith("/*"):
                total += n
                if "*/" not in s:
                    in_block = True
        else:
            if lang == "py" and in_doc:
                total += n
                if '"""' in s:
                    in_doc = False
                continue
            if s.startswith("#") and not s.startswith("#!"):
                total += n
            elif lang == "py" and s.startswith('"""'):
                total += n
                if s.count('"""') == 1:
                    in_doc = True
    return total


def diff_comment_bytes(text):
    """git diff/show output: comment bytes of code files, and bytes of code files."""
    total = 0
    code = 0
    lang = None
    for raw in text.split("\n"):
        if raw.startswith("diff --git "):
            parts = raw.split(" b/")
            lang = lang_of(parts[-1]) if len(parts) > 1 else None
            continue
        if not raw or raw[0] not in "+- ":
            continue
        if raw.startswith(("+++", "---")):
            continue
        if not lang:
            continue
        n = len(raw) + 1
        code += n
        s = raw[1:].strip()
        if (lang == "cpp" and (s.startswith("//") or s.startswith("/*") or s.startswith("*"))) or (
            lang != "cpp" and s.startswith("#")
        ):
            total += n
    return total, code


def classify_bash(cmd):
    """Returns (category, path or None, kind)."""
    c = cmd.strip()
    segs = [s.strip() for s in re.split(r"&&|;|\n", c) if s.strip()]
    head = ""
    for s in segs:
        if s.startswith(("cd ", "export ", "set ", "#")) or re.match(r"^[A-Z_]+=\S*$", s):
            continue
        head = s
        break
    head = re.sub(r"^(?:[A-Z_]+=\S+\s+)+", "", head)
    head = re.sub(r"^(?:timeout\s+\S+\s+|time\s+|nice\s+(?:-n\s*\d+\s+)?|env\s+(?:[A-Z_]+=\S+\s+)*)+", "", head)
    verb = head.split()[0] if head.split() else ""
    verb = verb.lstrip("(")
    # the repo root in a leading `cd` says nothing about what the command reads
    scan = re.sub(r"(^|&&|;|\n)\s*cd\s+\S+", " ", c)
    path = None
    for m in PATHTOK.finditer(" " + scan):
        cand = m.group(1)
        if category_of_path(cand) not in ("other_path", "unknown", "outside"):
            path = cand
            break
    BASH_VERBS[verb] += 1
    if verb == "git":
        sub = head.split()[1] if len(head.split()) > 1 else ""
        if "-C" == sub and len(head.split()) > 3:
            sub = head.split()[3]
        if sub in ("diff", "show"):
            return "cmd/git_diff_show", path, "diff"
        if sub in ("log", "blame", "grep", "shortlog", "rev-list"):
            return "cmd/git_log_grep", path, "grep" if sub == "grep" else "git"
        return "cmd/git_other", path, "git"
    if verb in ("cat", "sed", "head", "tail", "grep", "egrep", "rg", "awk", "nl", "cut", "less", "wc", "ls", "find", "sort", "diff"):
        cat = category_of_path(path) if path else "cmd/read_unknown"
        return ("read/" + cat) if path else cat, path, ("grep" if verb in ("grep", "egrep", "rg") else "read")
    if re.search(r"\b(cmake|ctest|make|ninja|g\+\+|clang\+\+|gate\.sh|gate_extra\.sh|clang-format)\b", head):
        return "cmd/build_test", path, "other"
    if re.search(r"fastchess|rating\.sh|nohup|_sprt\.sh|_screen\.sh|_cost\.sh", head):
        return "cmd/run_start", path, "other"
    if verb in ("python", "python3") or "python" in verb:
        return "cmd/python", path, "other"
    if verb in ("until", "while", "sleep", "pgrep", "ps", "kill", "pkill", "tail", "uptime", "free") or "loadavg" in head:
        return "cmd/watch", path, "other"
    if verb in ("for",):
        if re.search(r"\b(cat|sed|head|tail|grep|awk|git show)\b", c) and path:
            return "read/" + category_of_path(path), path, "read"
        return "cmd/loop", path, "other"
    if verb in ("echo", "printf", "true", "date", "pwd", "which", "command", "mkdir", "cp", "mv", "rm", "touch", "tar", "sha256sum", "du", "df", "stat", "ln", "chmod", "tee"):
        return "cmd/shell_misc", path, "other"
    if verb.startswith(("./build", "build", ".ref-builds", "bash", "sh")) or "/build" in verb or "chesso" in verb:
        return "cmd/binaries_scripts", path, "other"
    if verb.startswith(("adocs/data", "./adocs/data", "tools/", "./tools/", ".tuning", "./.tuning", "tests/", "./tests/")):
        return "cmd/binaries_scripts", path, "other"
    return "cmd/other", path, "other"


def text_of(content):
    if isinstance(content, str):
        return content
    if isinstance(content, list):
        out = []
        for x in content:
            if isinstance(x, dict) and x.get("type") == "text":
                out.append(x.get("text", ""))
        return "\n".join(out)
    return ""


STEP = re.compile(r"\bS\d{3}\b")
BASH_VERBS = Counter()
CAL = defaultdict(list)
REG = []
MODE = Counter()
FILES = Counter()


def repo_rel(p):
    p = (p or "").strip("'\"")
    if "/ws/chesso" in p:
        p = STRIP.sub("", p)
    return WT.sub("", p.lstrip("./"))


def rd_mult(model):
    return 0.05 if model == "claude-opus-5-5" else 0.1


def process(path, meta):
    """One transcript. Items land in `pending` and join the context at the next request."""
    reqs = {}
    order = []
    tool_uses = {}
    ctx = Counter()  # tokens in context by category
    ctx_comment = Counter()  # comment tokens in context by category
    pending = Counter()
    pending_comment = Counter()
    raw_by = Counter()
    w_by = Counter()
    raw_comment = Counter()
    w_comment = Counter()
    item_tokens = Counter()
    item_comment_tokens = Counter()
    n_results = Counter()
    tool_counts = Counter()
    authored = Counter()
    brief_bytes = 0
    steps = Counter()
    compactions = 0
    first_user_seen = False
    excluded = False
    totals = Counter()
    n_req = 0
    last_output = 0
    base_tokens = None
    per_read = []
    first_ts = None
    last_ts = None
    models = Counter()

    single_result = []
    last_code_read = [None]

    def add_item(cat, nbytes, comment=0):
        tok = nbytes / 4
        if cat.startswith(("Read:", "Bash:", "Grep:", "cmd/")):
            single_result.append((cat, nbytes))
        ctok = comment / 4
        pending[cat] += tok
        pending_comment[cat] += ctok
        item_tokens[cat] += tok
        item_comment_tokens[cat] += ctok
        n_results[cat] += 1

    records = []
    with open(path, errors="replace") as fh:
        for line in fh:
            try:
                d = json.loads(line)
            except ValueError:
                continue
            records.append(d)
    # keep the richest usage per request: a request's records repeat it
    usage_of = {}
    for d in records:
        if d.get("type") == "assistant":
            rid = d.get("requestId") or (d.get("message") or {}).get("id")
            u = (d.get("message") or {}).get("usage")
            if rid and u:
                prev = usage_of.get(rid)
                if prev is None or u.get("output_tokens", 0) >= prev.get("output_tokens", 0):
                    usage_of[rid] = u
    seen_req = set()
    for d in records:
        t = d.get("type")
        ts = d.get("timestamp")
        if ts:
            first_ts = first_ts or ts
            last_ts = ts
        if t == "system" and d.get("subtype") == "compact_boundary":
            ctx.clear()
            ctx_comment.clear()
            pending.clear()
            pending_comment.clear()
            compactions += 1
            continue
        if t == "attachment":
            a = d.get("attachment") or {}
            # only what reaches the model as a reminder; the record's own framing is not sent
            atype = a.get("type")
            if atype in ("file", "edited_text_file", "instructions", "skill_listing", "nested_memory", "plan_file_reference"):
                add_item("attachments", len(json.dumps(a)))
            continue
        if t == "user":
            msg = d.get("message") or {}
            c = msg.get("content")
            if isinstance(c, str):
                if not first_user_seen:
                    first_user_seen = True
                    brief_bytes += len(c)
                    steps.update(STEP.findall(c[:2000]))
                    if "read-only study of the chesso" in c[:300]:
                        excluded = True
                    add_item("prompt_first", len(c))
                else:
                    add_item("user_text", len(c))
                continue
            if isinstance(c, list):
                for x in c:
                    if not isinstance(x, dict):
                        continue
                    if x.get("type") == "text":
                        if not first_user_seen:
                            first_user_seen = True
                            brief_bytes += len(x.get("text", ""))
                            steps.update(STEP.findall(x.get("text", "")[:2000]))
                            if "read-only study of the chesso" in x.get("text", "")[:300]:
                                excluded = True
                            add_item("prompt_first", len(x.get("text", "")))
                        else:
                            add_item("user_text", len(x.get("text", "")))
                    elif x.get("type") == "tool_result":
                        tu = tool_uses.get(x.get("tool_use_id"))
                        body = text_of(x.get("content"))
                        n = len(body.encode("utf-8", "replace"))
                        if not tu:
                            add_item("result_unmatched", n)
                            continue
                        name, inp = tu
                        if name == "Read":
                            fp = inp.get("file_path", "")
                            cat = category_of_path(fp)
                            cm = comment_bytes(body, lang_of(fp)) if cat in CODE_CATS else 0
                            add_item("Read:" + cat, n, cm)
                            if cat in CODE_CATS:
                                mode = "Read window" if (inp.get("offset") or inp.get("limit")) else "Read whole"
                                MODE[(mode, "bytes")] += n
                                MODE[(mode, "comment")] += cm
                                MODE[(mode, "calls")] += 1
                                last_code_read[0] = (cm, n - cm)
                                FILES[(repo_rel(fp), "bytes")] += n
                                FILES[(repo_rel(fp), "comment")] += cm
                            if cat in CODE_CATS:
                                per_read.append((cat, n, cm))
                        elif name in ("Grep", "Glob"):
                            fp = inp.get("path") or ""
                            cat = category_of_path(fp) if fp else "unknown"
                            lg = lang_of(inp.get("glob") or "") or lang_of(fp) or ("cpp" if cat == "src" else None)
                            cm = comment_bytes(body, lg, grep=True) if cat in CODE_CATS else 0
                            add_item(name + ":" + cat, n, cm)
                        elif name == "Bash":
                            cmd = inp.get("command", "")
                            cat, pth, kind = classify_bash(cmd)
                            cm = 0
                            if kind == "diff":
                                cm, code = diff_comment_bytes(body)
                                add_item(cat, n, cm)
                            elif kind in ("read", "grep"):
                                base = cat.replace("read/", "")
                                if base in CODE_CATS:
                                    lg = lang_of(pth) or ("cpp" if base == "src" else "hash" if base == "root_scripts" else None)
                                    if lg is None and base in ("tests", "tools"):
                                        lg = "cpp"
                                    cm = comment_bytes(body, lg, grep=(kind == "grep"))
                                add_item("Bash:" + cat, n, cm)
                                if base in CODE_CATS:
                                    mode = "Bash grep" if kind == "grep" else ("Bash sed window" if re.search(r"\bsed\s+-n|\bhead\b|\btail\b", cmd) else "Bash cat/other")
                                    MODE[(mode, "bytes")] += n
                                    MODE[(mode, "comment")] += cm
                                    MODE[(mode, "calls")] += 1
                                    last_code_read[0] = (cm, n - cm)
                                    FILES[(repo_rel(pth), "bytes")] += n
                                    FILES[(repo_rel(pth), "comment")] += cm
                            else:
                                add_item("Bash:" + cat, n)
                        elif name in ("Agent", "Task", "SendMessage"):
                            add_item("agent_report", n)
                        else:
                            add_item("tool_other", n)
            continue
        if t == "assistant":
            msg = d.get("message") or {}
            rid = d.get("requestId") or msg.get("id")
            models[msg.get("model")] += 1
            if rid and rid not in seen_req:
                seen_req.add(rid)
                u = usage_of.get(rid) or {}
                inp = u.get("input_tokens", 0)
                cc = u.get("cache_creation_input_tokens", 0)
                cr = u.get("cache_read_input_tokens", 0)
                out = u.get("output_tokens", 0)
                ccd = u.get("cache_creation") or {}
                e5 = ccd.get("ephemeral_5m_input_tokens", 0)
                e1 = ccd.get("ephemeral_1h_input_tokens", 0)
                think = (u.get("output_tokens_details") or {}).get("thinking_tokens", 0)
                model = msg.get("model") or ""
                rd = rd_mult(model)
                wr = (1.25 * e5 + 2 * e1) / (e5 + e1) if (e5 + e1) else 1.25
                ctx_total = inp + cc + cr
                n_req += 1
                if base_tokens is None:
                    base_tokens = ctx_total
                non_assistant = sum(v for k, v in pending.items() if not k.startswith(("assistant", "agent_prompts")))
                new_total = non_assistant + last_output
                if len(single_result) == 1 and n_req > 1:
                    cat_, nb_ = single_result[0]
                    actual = cc + inp - last_output
                    if nb_ >= 4000 and actual > 0 and actual < nb_:
                        CAL[cat_].append(nb_ / actual)
                        if nb_ >= 20000:
                            CAL["LARGE:" + cat_].append(nb_ / actual)
                        if last_code_read[0] is not None and sum(last_code_read[0]) == nb_:
                            REG.append((last_code_read[0][0], last_code_read[0][1], actual))
                last_code_read[0] = None
                single_result.clear()
                old_total = max(1.0, ctx_total - new_total)
                miss = min(1.0, max(0.0, (cc + inp - new_total)) / old_total)
                old_w = miss * wr + (1 - miss) * rd
                for cat in set(ctx) | set(pending):
                    raw_by[cat] += ctx[cat] + pending[cat]
                    w_by[cat] += ctx[cat] * old_w + pending[cat] * wr
                    raw_comment[cat] += ctx_comment[cat] + pending_comment[cat]
                    w_comment[cat] += ctx_comment[cat] * old_w + pending_comment[cat] * wr
                ctx.update(pending)
                ctx_comment.update(pending_comment)
                pending.clear()
                pending_comment.clear()
                totals["input"] += inp
                totals["cache_write"] += cc
                totals["cache_write_5m"] += e5
                totals["cache_write_1h"] += e1
                totals["cache_read"] += cr
                totals["output"] += out
                totals["thinking"] += think
                totals["raw_context"] += ctx_total
                totals["weighted_input_side"] += inp + wr * cc + rd * cr
                totals["weighted_output"] += 5 * out
                totals["weighted_total"] += inp + wr * cc + rd * cr + 5 * out
                last_output = out
                if think:
                    pending["assistant_thinking"] += think
                    item_tokens["assistant_thinking"] += think
            for x in msg.get("content") or []:
                if not isinstance(x, dict):
                    continue
                if x.get("type") == "tool_use":
                    tool_uses[x.get("id")] = (x.get("name"), x.get("input") or {})
                    tool_counts[x.get("name")] += 1
                    inp_txt = x.get("input") or {}
                    if x.get("name") in ("Edit", "Write", "MultiEdit"):
                        fp = inp_txt.get("file_path", "")
                        body = inp_txt.get("new_string") or inp_txt.get("content") or ""
                        cat = category_of_path(fp)
                        if cat in CODE_CATS:
                            authored[cat + ":bytes"] += len(body)
                            authored[cat + ":comment_bytes"] += comment_bytes(body, lang_of(fp))
                        else:
                            authored[cat + ":bytes"] += len(body)
                        add_item("assistant_edit_inputs", len(json.dumps(inp_txt)))
                    elif x.get("name") in ("Agent", "Task"):
                        add_item("agent_prompts_sent", len(json.dumps(inp_txt)))
                    else:
                        add_item("assistant_tool_inputs", len(json.dumps(inp_txt)))
                elif x.get("type") == "text":
                    add_item("assistant_text", len(x.get("text", "")))
    row = dict(
        file=os.path.relpath(path, ROOT),
        kind=(meta or {}).get("agentType", "main"),
        desc_steps=sorted(set(STEP.findall((meta or {}).get("description", "")))),
        brief_steps=[s for s, _ in steps.most_common(3)],
        desc_kind=desc_kind((meta or {}).get("description", "")),
        first=first_ts,
        last=last_ts,
        requests=n_req,
        compactions=compactions,
        base_tokens=base_tokens or 0,
        brief_bytes=brief_bytes,
        totals=dict(totals),
        raw_by=dict(raw_by),
        w_by=dict(w_by),
        raw_comment=dict(raw_comment),
        w_comment=dict(w_comment),
        item_tokens=dict(item_tokens),
        item_comment_tokens=dict(item_comment_tokens),
        n_results=dict(n_results),
        tools=dict(tool_counts),
        authored=dict(authored),
        models=dict(models),
        excluded=excluded,
    )
    return row


def desc_kind(desc):
    d = desc.lower()
    if re.search(r"fast check|fast-check|review|audit", d):
        return "review"
    if re.search(r"explor|study|survey|census|read|find|research", d):
        return "explore"
    if re.search(r"\bclose|closing|land|record|finish|complete", d):
        return "close"
    if re.search(r"\bs\d{3}\b|implement|step|build|screen|fix", d):
        return "implement"
    return "other"


def main():
    paths = []
    for p in sorted(glob.glob(ROOT + "/*.jsonl")):
        paths.append((p, None))
    for p in sorted(glob.glob(ROOT + "/*/subagents/*.jsonl")):
        mp = p[: -len(".jsonl")] + ".meta.json"
        meta = None
        if os.path.exists(mp):
            try:
                meta = json.load(open(mp))
            except ValueError:
                meta = None
        paths.append((p, meta or {"agentType": "subagent?"}))
    with open(os.path.join(OUT, "transcript_rows.jsonl"), "w") as out:
        for p, meta in paths:
            r = process(p, meta)
            out.write(json.dumps(r) + "\n")
    print("TRANSCRIPTS-DONE", len(paths))
    import statistics as st
    print("top bash verbs", BASH_VERBS.most_common(30))
    # real tokens = a * comment bytes + b * other bytes + c, ordinary least squares
    n = len(REG)
    if n > 10:
        X = [(c, o, 1.0) for c, o, _ in REG]
        y = [t for _, _, t in REG]
        A = [[sum(x[i] * x[j] for x in X) for j in range(3)] for i in range(3)]
        B = [sum(x[i] * v for x, v in zip(X, y)) for i in range(3)]
        for i in range(3):
            piv = A[i][i]
            A[i] = [v / piv for v in A[i]]
            B[i] /= piv
            for k in range(3):
                if k != i:
                    f = A[k][i]
                    A[k] = [p - f * q for p, q in zip(A[k], A[i])]
                    B[k] -= f * B[i]
        print(f"regression n={n}: tokens per comment byte {B[0]:.3f} (={1/B[0]:.2f} bytes/token), per other byte {B[1]:.3f} (={1/B[1]:.2f} bytes/token), overhead {B[2]:.0f}")
        json.dump(dict(n=n, per_comment_byte=B[0], per_other_byte=B[1], overhead=B[2]), open(os.path.join(OUT, "token_regression.json"), "w"))
    json.dump({f"{a}|{b}": v for (a, b), v in FILES.items()}, open(os.path.join(OUT, "read_files.json"), "w"))
    print("how code was read:")
    for mode in sorted({k[0] for k in MODE}):
        print(f"  {mode:18s} calls {MODE[(mode,'calls')]:6d} bytes {MODE[(mode,'bytes')]:11d} comment bytes {MODE[(mode,'comment')]:11d} ({100*MODE[(mode,'comment')]/max(1,MODE[(mode,'bytes')]):.0f}%)")
    json.dump({f"{a}|{b}": v for (a, b), v in MODE.items()}, open(os.path.join(OUT, "read_modes.json"), "w"))
    print("bytes per real token, one result arriving alone, >=4 kB, no miss:")
    fam = defaultdict(list)
    for k, v in CAL.items():
        fam[k.replace("Bash:read/", "").replace("Read:", "").replace("Bash:", "")] += v
    for k, v in sorted(fam.items(), key=lambda kv: -len(kv[1]))[:22]:
        if len(v) >= 8:
            print(f"  {k:24s} n={len(v):5d} median {st.median(v):.2f}")


if __name__ == "__main__":
    main()
