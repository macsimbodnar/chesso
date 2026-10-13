#!/usr/bin/env python3
"""S278: every `add <sym>@gottpoff(%rip), %reg` whose flags a later
instruction reads.

The linker rewrites that add into a `lea` when it relaxes initial-exec TLS to
local-exec in an executable, and a lea sets no flags, so a flag reader after it
reads whatever the instruction before the add left. In the sanitizer build GCC
13.3 placed UBSan's null check on `&eval_trace::sink` exactly there, and the
check fired on a valid address.

From each such add the scan walks forward through the instructions that
neither write the flags nor transfer control -- moves, lea, and the nops and
prefixes `-Wa,-mbranches-within-32B-boundaries` pads with (cmake/arch.cmake
sets it), which put a `nopw` between the add and its `je` -- until it meets:

  a flag reader       j<cc>, set<cc>, cmov<cc>, adc, sbb, rcl, rcr, pushf,
                      lahf: counted as a site;
  a flag writer       add, sub, cmp, test, and, or, xor, inc, dec, neg, the
                      shifts and rotates, bit scans, comis, ptest...: the add's
                      flags are dead, nothing counted;
  call, ret, ud2      the flags do not survive it, nothing counted;
  anything else       jmp, which carries the flags to its target, or an
                      instruction this list does not classify: counted as
                      unresolved and printed, never assumed safe.

    python3 adocs/data/S278_tls_flag_scan.py $(find build-sanitize -name '*.o')

Prints one line per site and per unresolved walk, then
`<sites> flag-reading uses of a GOTTPOFF add, <unresolved> unresolved, in <n>
objects`. Only `0 ... 0 unresolved` says that no object in the set reads
flags off such an add. Needs binutils' objdump. Reads objects, not linked
binaries: the relocation is what names the add, and the link has already
rewritten it.
"""
import re
import subprocess
import sys

FUNCTION = re.compile(r"^[0-9a-f]+ <.*>:$")
INSN = re.compile(r"^\s*([0-9a-f]+):\s+(.*)$")
PREFIXES = {"cs", "ds", "ss", "es", "fs", "gs", "data16", "addr32", "lock",
            "rep", "repz", "repnz", "repe", "repne", "notrack", "bnd"}

READER = re.compile(
    r"^(j(?!mp$|rcxz$|ecxz$)[a-z]+|set[a-z]+|cmov[a-z]+|adc[bwlq]?|sbb[bwlq]?"
    r"|rc[lr][bwlq]?|pushf[wq]?|lahf|adcx|adox)$")
WRITER = re.compile(
    r"^((add|sub|cmp|test|and|or|xor|inc|dec|neg|shl|sal|shr|sar|rol|ror|imul"
    r"|mul|div|idiv|bt|bts|btr|btc|bsf|bsr|popcnt|lzcnt|tzcnt|cmpxchg|xadd"
    r"|shld|shrd|scas|cmps)[bwlq]?"
    r"|andn|bextr|blsi|blsmsk|blsr|bzhi|v?u?comis[sd]|v?ptest|vtestp[sd]|sahf"
    r"|popf[wq]?|stc|clc|cmc)$")
NEUTRAL = re.compile(
    r"^(nop[wl]?|xchg[bwlq]?|mov[a-z]*|lea[wlq]?|push[wlq]?|pop[wlq]?"
    r"|bswap[lq]?|not[bwlq]?|cbtw|cwtl|cltq|cqto|cltd|cwtd|endbr64"
    r"|prefetch[a-z0-9]*|shlx|shrx|sarx|rorx|pdep|pext|mulx)$")
STOP = re.compile(r"^(call[q]?|ret[q]?|ud2|hlt|int3)$")


def mnemonic(text):
    for token in text.split():
        if token in PREFIXES or token.startswith("rex"):
            continue
        return token
    return ""


def functions(listing):
    current = []
    for line in listing:
        if FUNCTION.match(line):
            if current:
                yield current
            current = []
            continue
        m = INSN.match(line)
        if m:
            current.append((m.group(1), m.group(2)))
    if current:
        yield current


sites = 0
unresolved = 0
for obj in sys.argv[1:]:
    listing = subprocess.run(["objdump", "-drw", "--no-show-raw-insn", obj],
                             capture_output=True, text=True,
                             check=True).stdout.splitlines()
    name = obj.rsplit("/", 1)[-1]
    for insns in functions(listing):
        for i, (addr, text) in enumerate(insns):
            if mnemonic(text) not in ("add", "addq") or \
               "R_X86_64_GOTTPOFF" not in text:
                continue
            symbol = text.split("R_X86_64_GOTTPOFF")[1].strip()
            for _, later in insns[i + 1:]:
                op = mnemonic(later)
                if READER.match(op):
                    sites += 1
                    print(f"{name} 0x{addr}: {symbol} -> {op}")
                    break
                if WRITER.match(op) or STOP.match(op):
                    break
                if NEUTRAL.match(op):
                    continue
                unresolved += 1
                print(f"{name} 0x{addr}: {symbol} -> unresolved at '{op}'")
                break

print(f"{sites} flag-reading uses of a GOTTPOFF add, {unresolved} unresolved,"
      f" in {len(sys.argv) - 1} objects")
