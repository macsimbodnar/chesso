# Lynx pull requests, banded by CCRL Blitz rating — S181, 2026-09-11

Every Lynx figure the pending steps and DEC-087 cite, mapped to the merge date
of its pull request, the two releases that date falls between, and the CCRL
Blitz 1CPU rating of each of those releases. `2026-09-04_plan_review-F02` is
the finding: DEC-087 (b) separated the correction-history family on "sub-3000
evidence" against "only above ~3100", and the one figure it called sub-3000 was
measured **above** the band that demoted the other two, so the criterion
separated nothing.

## Provenance, and it is not the same reading the step file quoted

**Ratings**: `https://computerchess.org.uk/ccrl/404/rating_list_all.html`,
**computed 2026-09-05 with Bayeselo over 2'106'571 games**, fetched here on
**2026-09-11** with `curl` and a browser user-agent (`WebFetch` returns HTTP
403 on that host). 1CPU rows, which are the unsuffixed ones.

The list is recomputed continually and it moved between readings. Against the
2026-08-28 reading quoted in `adocs/plan_todo/S181_lynx_band_correction.md`:
1.11.0 **3360** against 3364, 1.10.0 **3291** against 3293, 1.9.0 **3224**
against 3226, 1.8.0 **3138** against 3139, 1.6.0 **2925** against 2926, 1.5.0
**2818** against 2819; 1.7.0 **3119** both times. So the drift over fourteen
days is 0 to 4 points and no band below changes because of it -- which is the
reason to record the read date rather than the number alone.

**Release dates**: `https://api.github.com/repos/lynx-chess/Lynx/releases`,
`published_at`, fetched 2026-09-11. **Merge dates**:
`https://api.github.com/repos/lynx-chess/Lynx/pulls/<n>`, `merged_at`, same
day, cached as `S181_lynx_prs.tsv`.

Read as an API record and a rating table. No source file, no evaluation table
and no test data was fetched from Lynx (DEC-016).

## The release timeline

Ratings as above; **not on the list** means CCRL has no row for that release,
so it cannot bracket anything and the nearest rated release is named instead.

| release | published | CCRL Blitz 1CPU |
|---|---|---|
| v1.0.0 | 2023-11-16 | not on the list |
| v1.0.1 | 2023-11-20 | 2430 |
| v1.1.0 | 2023-12-14 | 2420 |
| v1.2.0 | 2024-01-11 | not on the list |
| v1.3.0 | 2024-02-04 | 2653 |
| v1.4.0 | 2024-03-21 | 2706 |
| v1.5.0 | 2024-06-09 | 2818 |
| v1.5.1 | 2024-06-21 | not on the list |
| v1.6.0 | 2024-08-15 | 2925 |
| v1.7.0 | 2024-10-04 | 3119 |
| v1.8.0 | 2024-12-20 | 3138 |
| v1.9.0 | 2025-03-11 | 3224 |
| v1.9.1 | 2025-04-05 | not on the list |
| v1.10.0 | 2025-06-29 | 3291 |
| v1.11.0 | 2025-10-01 | 3360 |

Lynx's own README table is the cross-check and agrees to within a few points
(it gives 3144 / 3225 / 3293 for v1.8.0 / v1.9.0 / v1.10.0 against this list's
3138 / 3224 / 3291), which settles the "band caution" S097 recorded on
2026-08-19 and could not resolve: **the README was right and the repository's
DEC-087-era banding of that Lynx era was about 300 low.**

## The pull requests

`band` is the rating range the two bracketing releases give. Where a
bracketing release is not on the list, the range runs from the nearest rated
release before the merge.

| PR | title | merged | between | band | cited by | the band word it replaces |
|---|---|---|---|---|---|---|
| #512 | Add basic LMP | 2023-11-24 | v1.0.1 - v1.1.0 | **2420 - 2430** | S109, plan.md | "~2600" -- the real band is 180 points *lower*, which strengthens the claim |
| #613 | Take history into consideration for LMR | 2024-01-15 | v1.2.0 (unrated) - v1.3.0 | **2420 - 2653** | S098 (b) | "~2600" -- holds; shipped in v1.3.0 at 2653 |
| #634 | Add capture history | 2024-02-02 | v1.2.0 (unrated) - v1.3.0 | **2420 - 2653** | S023, DEC-087 (b) | "~2600" -- holds as a band, but see the note below: it **merged** |
| #637 | Stop clearing quiet history | 2024-02-03 | v1.2.0 (unrated) - v1.3.0 | **2420 - 2653** | S093 (done), plan.md | none |
| #733 | Futility pruning | 2024-05-14 | v1.4.0 - v1.5.0 | **2706 - 2818** | S109, plan.md | none stated |
| #971 | LMR: clamp history reductions to [-1,1] | **not merged** | -- | no release carries it; by number it sits between #733 (2024-05) and #1135 (2024-10), which is v1.5.0 to v1.7.0, **2818 - 3119** -- inferred from the number, not from a date | S098 (b) | none |
| #1135 | Improving: LMR, `if (!improving) ++reduction;` | 2024-10-31 | v1.7.0 - v1.8.0 | **3119 - 3138** | S098 (c), S108 | "v1.8.0 high-2800s" -- **wrong by ~300** |
| #1203 | Add node time management | 2024-11-27 | v1.7.0 - v1.8.0 | **3119 - 3138** | S132 | none stated |
| #1206 | Node time management: Base 2.4 scale 1.65 | 2024-11-27 | v1.7.0 - v1.8.0 | **3119 - 3138** | S132 | none stated |
| #1233 | LMR: reduce more on cutnode | 2024-12-05 | v1.7.0 - v1.8.0 | **3119 - 3138** | S098 (c) | "v1.8.0, 2024, high-2800s" -- **wrong by ~300** |
| #1230 | LMR: increase pv min moves 2 | 2024-12-08 | v1.7.0 - v1.8.0 | **3119 - 3138** | S098 (c) | "v1.8.0" under Sub-3000 evidence -- **wrong** |
| #1476 | Add TTPV - depth offset 2 + LMR increase on no ttPv | 2025-02-10 | v1.8.0 - v1.9.0 | **3138 - 3224** | S098 (deferred) | already filed 3100+, correct |
| #1512 | LMR: split base and history factor, fractional LMR | 2025-02-23 | v1.8.0 - v1.9.0 | **3138 - 3224** | S098 (deferred) | already filed 3100+, correct |
| #1529 | LMR TT capture | 2025-03-04 | v1.8.0 - v1.9.0 | **3138 - 3224** | S098 (c) | "v1.9.0" at the ~3000-3100 boundary -- **it is 3138-3224** |
| #1535 | LMR: Deeper/shallower | 2025-03-05 | v1.8.0 - v1.9.0 | **3138 - 3224** | S098 (d) | "v1.9.0, 2025-03" at the boundary -- **it is 3138-3224** |
| #1662 | Pawn correction history / corrhist | 2025-04-15 | v1.9.1 (unrated) - v1.10.0 | **3224 - 3291** | S099, DEC-087 (b) | "~2850" -- **wrong by ~380**, and this is F02's whole point |
| #1663 | Pawn corrhist - no king in hash | 2025-04-15 | v1.9.1 (unrated) - v1.10.0 | **3224 - 3291** | S099 | same |
| #1731 | Singular Extensions (SE) | 2025-06-08 | v1.9.1 (unrated) - v1.10.0 | **3224 - 3291** | S097 | "sub-3000" grouping of the Lynx SE block -- **it is 3224-3291** |
| #1743 | SE: negative extension | 2025-06-10 | v1.9.1 (unrated) - v1.10.0 | **3224 - 3291** | S097 | already filed 3100+, correct |
| #1751 | SE: multicut with `singularScore` | 2025-06-11 | v1.9.1 (unrated) - v1.10.0 | **3224 - 3291** | S097 | none |
| #1999 | Allow static eval correction and improving on pv nodes | 2025-08-26 | v1.10.0 - v1.11.0 | **3291 - 3360** | S099 | none |

## The capture-history claim does not survive, and it is not a band problem

DEC-087 (b) reads *"Capture history failed four SPRTs at ~2600 (Lynx)"*, and
S023 carried *"Lynx failed four SPRTs on it at ~2600 (-35.8 to -11.1)"*.
Neither names a pull request, and the 2026-09-04 literature check found no
source for the four (row A13 traced the Weiss figure and only that).

What the API record says: **Lynx merged capture history**, pull request #634,
"Add capture history", 2024-02-02, into v1.3.0 at **2653**. So the feature was
adopted at that band rather than rejected at it. A title search over the
repository's capture-history pull requests
(`https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+type:pr+capture+history+in:title`,
19 results, 2026-09-11) shows the closed-unmerged ones are **later
refinements** of a feature already in the tree -- SEE thresholds keyed on
capture history, quiescence updates, bad-capture pruning, threat terms -- not
four attempts at the feature itself, and their dates are 2024-09 to 2026-03,
which is the 2925-and-above era.

Two of those four numbers may well exist somewhere; what does not exist is any
route from this tree to them. **The claim is withdrawn as unverified**
(DEC-176). The demotion of S023 does not depend on it: what supports it is the
figure the literature check *did* trace, Weiss pull request #428, "Capture
History", 2021-06-04, **-4.17 +/- 4.83** at 10+0.1 over 9088 games against
**+3.66 +/- 3.29** at 60+0.6 over 15936, with the author's own reading "hurts
STC, but LTC shows decent gain" -- measured by an engine CCRL Blitz rates at
**3055** for v1.2, which is above chesso's target and above every band in the
table here.
