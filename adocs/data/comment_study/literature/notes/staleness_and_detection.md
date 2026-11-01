# Code-comment staleness, LLM-written comments, and what detects or prevents them

As of 2026-11-01. Reading status of each source, because it changes how far a number can be trusted:

- `[FT]` full text extracted from the PDF and read directly.
- `[HTML]` arXiv or ar5iv HTML read through a fetch tool that summarises. Numbers are as that tool returned them; arithmetic was cross-checked where the note says so.
- `[ABS]` abstract or landing page only.
- `[VIA x]` number reported by paper x, which was opened. The primary was not opened.
- `not opened` source could not be read. Its numbers are not quoted.

`Human` means the comments studied were human-written or the study predates LLM authorship. `LLM` means LLM-written or LLM-processed. `VENDOR` means an interested party.

## Q1. How often comments go stale, what predicts it, how long inconsistencies survive

### Takeaway
Co-change is rare and staleness is common: 13-20% of code changes touch a comment (Java, Human), and at least 10.6% of documented functions in maintained projects contain a confirmed incorrect statement (4 languages, Human). No study tests comment length or comment age as a predictor of falsehood, so the reader's result is neither confirmed nor contradicted directly. Indirect evidence on age leans against it for long-lived human repositories, and the length result needs normalising by claim count before it means more than "long blocks hold more claims".

### Cited Findings

#### Co-change rates (all Human, all pre-LLM)
- **Wen et al., ICPC 2019.** Mining plus manual taxonomy. 1,500 Java GitHub projects (at least 500 commits, at least 10 stars, top by stars), 3,323,198 commits, 1.3 billion AST changes. Method comment changes in 7% of cases, class comments in 13%, so 13-20% of code changes trigger a comment change. The authors state this "does not imply" the other ~80% are inconsistent. [FT] — [Wen 2019](https://www.inf.usi.ch/faculty/lanza/PUBS/P/Wen2019a.pdf)
- **Wen, by change type.** Variable Declaration MCC 0.69, Selection also high. Constructor MCC 0.06 vs class-comment CCC 0.83. Method Signature MCC 0.04 vs CCC 0.68. Statistically lowest chance of triggering an update: Array, Lambda, Iteration, Literal, Method Invocation, Name. Most other categories 0.1-0.2. [FT] — [Wen 2019](https://www.inf.usi.ch/faculty/lanza/PUBS/P/Wen2019a.pdf)
- **Wen, taxonomy.** 500 keyword-selected commits, 138 false positives, 362 real; 69 change types, 25 relevant to inconsistency. Application Logic 136, Code Design/Quality 80, Formatting 63, Others 56, Maintenance 17, Licence 10. Within Application Logic: 53 improved detail, 35 wrongly described logic, 25 updated for new logic. Refactoring is named as a cause (replaced magic number left in the comment, class-hierarchy changes leaving stale references). [FT] — [Wen 2019](https://www.inf.usi.ch/faculty/lanza/PUBS/P/Wen2019a.pdf)
- **Fluri et al. 2007, 2009. Primaries not opened.** As described by Wen: 23%, 52%, 43% of comment changes in ArgoUML, Azureus, JDT Core were due to code changes, 97% of those in the same revision; a follow-up on eight systems found 90% same-revision; newly added code barely got commented. As described by Ibrahim for the eight-system paper: 51-69% of comment changes driven by code changes. Different denominators, not a conflict. [VIA Wen, Ibrahim] — [Wen 2019](https://www.inf.usi.ch/faculty/lanza/PUBS/P/Wen2019a.pdf); [Ibrahim 2012](https://sailresearch.github.io/sail-website/data/pdfs/JSS_OnTheRelationshipBetweenCommentUpdatePracticesAndSoftwareBugs.pdf)

#### Prevalence of false or stale statements
- **DocPrism, ISSTA 2026.** Tool run on 1,991 developer-written function/doc pairs, 20 projects, Python, TypeScript, C++, Java; flagged pairs manually labelled. 211 confirmed incorrect: "at least 11%" (10.6%; Wilson 95% 9.3-12.0%, my calc). Lower bound because false negatives were not examined. Human. [FT] — [DocPrism](https://arxiv.org/pdf/2511.00215)
- **DocPrism, what the false statements were** (n=352 true inconsistencies): Functionality Mismatch 47%, Unimplemented Feature 30%, Parameter Mismatch 7%, Return Type Mismatch 5%, Identifier Mismatch 4%, Parameter Type Mismatch 3%, Unused Parameter 3%. C++ alone (n=67): Functionality 57%, Unimplemented 15%, Unused 15%, Return 6%, Identifier 5%, Parameter 3%. [FT] — [DocPrism](https://arxiv.org/pdf/2511.00215)
- **DOCER, Tan, Wagner, Treude, EMSE 2024.** README and wiki only; matches inside source comments were treated as false positives, so this says nothing direct about code comments. Top-1000 projects: 28.9% (265/918) currently hold an outdated code reference, 3.9% of references currently outdated, 82.3% (658/800) of projects had one at some point. Google projects: 5.4% and 29.7%. The paper's denominators differ between sections (991 vs 918). Human. [HTML] — [DOCER](https://ar5iv.labs.arxiv.org/html/2212.01479)
- **Hata et al., ICSE 2019.** 9,654,702 links in comments, 25,925 repos, 7 languages. 9.1% of 382,650 distinct links returned 404. Sample of 1,146 links: 7.7% ever revised. Human. [HTML] — [Hata 2019](https://ar5iv.labs.arxiv.org/html/1901.07440)
- **CoCC, 2024.** 49,648 Java code-comment pairs, about 16.8% labelled outdated by the code change. A dataset rate on 22 Java projects, not a population estimate. Human. [HTML] — [CoCC](https://arxiv.org/html/2403.00251v1)
- **LLM-generated comments start wrong rather than going stale:** see Q4, about a fifth for GPT-4 in Kang 2024.

#### Predictors other than length and age
- **Malik, Marin, primaries not opened.** As described by Ibrahim: developers update comments more on large and complex changes and on well-commented code; Malik's model is driven by change complexity, time of change, developer. No numbers given by Ibrahim. Human. [VIA Ibrahim] — [Ibrahim 2012](https://sailresearch.github.io/sail-website/data/pdfs/JSS_OnTheRelationshipBetweenCommentUpdatePracticesAndSoftwareBugs.pdf)
- **CoCC feature importance.** Random Forest Gini: top features are comment/code similarity, proportion of changed lines, token overlap, then method-invocation changes, return and rename changes. Comment type is a dataset split, not a feature. Human, just-in-time setting. [HTML] — [CoCC](https://arxiv.org/html/2403.00251v1)

#### Length: direct evidence on the reader's finding
- **No study found that regresses falsehood on comment length.** Text search of Wen for length, age, lifetime, survival returned only one anecdote (see Survival).
- **CoCC:** comment length was removed in feature filtering because it was highly correlated with other features, so it was not tested independently. Neutral. [HTML] — [CoCC](https://arxiv.org/html/2403.00251v1)
- **Kang 2024:** "we chose the methods with the longest comments both to ease the labeling process". No length analysis. The ~20% GPT-4 error rate is therefore a long-comment rate. LLM, Java. [HTML] — [Kang 2024](https://arxiv.org/html/2406.14836v1)
- **Panthaplackel 2021:** summary comments are the longest type (mean 13.3 subtokens vs @return 9.7, @param 8.4) and have the worst label noise (26-28% vs 6-15% and 14-16%). This concerns dataset labels, not false comments. Weak and indirect. Human. [HTML] — [Panthaplackel 2021](https://ar5iv.labs.arxiv.org/html/2010.01625)
- **Looks contradictory, is not comparable.** CodeRabbit: "Longer comments (top half by length) pass more often, with 53% precision compared to 34% for shorter comments." These are model-written review comments on 25 seeded PRs, split size not reported. VENDOR. — [CodeRabbit](https://www.coderabbit.ai/blog/gemini-3-for-code-related-tasks-the-dense-engineer)

#### Age: indirect evidence, leaning against "age does not matter" in long-lived repos
- **DOCER:** references currently outdated have been outdated for a mean 4.7 years (top-1000) and 4.2 years (Google). Stale references accumulate rather than get fixed. README and wiki, Human. [HTML] — [DOCER](https://ar5iv.labs.arxiv.org/html/2212.01479)
- **Bavota and Russo, MSR 2016:** 159 projects, over 600K commits. SATD count grows over time; even when fixed it persists over 1,000 commits on average. Human. [ABS] — [Bavota 2016](https://bia.unibz.it/esploro/outputs/conferenceProceeding/A-Large-Scale-Empirical-Study-on-Self-Admitted/991006853897901241)
- **Arafat and Riehle, ICSE NIER 2009:** comment density independent of team and project size but not of project age; correlation -0.9054 across 0-48 months. Density, not truth. Human. [FT] — [Arafat 2009](https://dirkriehle.com/wp-content/uploads/2009/02/icse-2009-nier-for-web.pdf)

#### Survival
- **DOCER:** historically fixed references had about a 55% chance (top-1000) and 45% (Google) of surviving past one month. Plus the 4.7 and 4.2 years above. [HTML] — [DOCER](https://ar5iv.labs.arxiv.org/html/2212.01479)
- **Wen:** no survival measure. One anecdote: a stale rename in alluxio fixed 20 days later. [FT] — [Wen 2019](https://www.inf.usi.ch/faculty/lanza/PUBS/P/Wen2019a.pdf)
- **Zampetti et al., MSR 2018:** 5 Java projects. 20-50% of SATD comments removed accidentally when whole classes or methods were dropped; 8% of removals acknowledged in commit messages. So a falling TODO count does not mean debt was paid. Human. [ABS] — [Zampetti 2018](https://aserebre.win.tue.nl/MSR2018-Fiorella.pdf)
- **Maldonado et al., ICSME 2017: not opened** (404). Numbers not quoted.

### Inferences
- The reader's counts: 9/36 vs 2/104. Fisher two-sided p = 7.8e-5, odds ratio 17.0, risk ratio 13.0. Wilson 95%: 25.0% [13.8, 41.1] vs 1.9% [0.5, 6.7]. Overall 11/140 = 7.9% [4.4, 13.5]. My calculations.
- The signal is real at block level, but a block-level rate rises with claim count even if each claim is equally likely to be false. Per-claim risk ratio = 13 / r, where r is the mean claim ratio between the strata: r=4 gives 3.25, r=8 gives 1.6, r=13 gives 1.0. A 1,200-byte cut can easily produce r above 4. Count claims or bytes in both strata before concluding that long blocks are worse per claim. If the 1,200 cut was chosen after seeing the data, the p-value is optimistic.
- The overall 7.9% sits near DocPrism's 10.6% lower bound for human code. The 25% stratum's interval contains Kang's ~20% long-comment rate for GPT-4. Different definitions, so suggestive only.
- The age null may be a range restriction. Human-repo evidence on accumulation runs over years. A young repository, or one whose agents rewrite comments often, has little age variance and little time for accumulation. Age cannot be excluded from this one study.
- 77% of confirmed false statements in DocPrism are behavioural (functionality, unimplemented feature); about 19-22% are mechanical (names, parameters, return types). For rule writing: symbol checks address the minority class.
- Long blocks may simply be where agents put behavioural claims. That would fit both the reader's result and DocPrism's mix, but it is untested.

### Gaps
- No study of falsehood rate against comment length, per claim or per block, in any language.
- No study of falsehood rate against comment age at comment level. Existing age data covers READMEs, links, SATD and density.
- No study of staleness rates in repositories maintained mostly by LLM agents. Everything above is human or single-shot LLM generation.
- Fluri primaries not opened: [Merlin records](https://www.merlin.uzh.ch/publication/show/142) have moved and the RERO record returned 410.

## Q2. Measured harm of stale comments for humans and LLMs

### Takeaway
Humans: only correlational evidence, with a conditional and small effect, plus one good controlled result that low-quality explanations lower accuracy while raising confidence. LLMs: wrong comments hurt reasoning and generation tasks in controlled but mostly artificial experiments; one strong null on vulnerability detection with current frontier models. The most on-point study (ASE 2026) reports no numbers in what could be opened.

### Cited Findings

#### Humans
- **Ibrahim et al., JSS 2012.** Bug-prediction models on FreeBSD, PostgreSQL (C), Eclipse (Java). Adding comment-update practices with an interaction term cut prediction error by a mean 26.8% (range 9.3-56.3%) for the change-based model and 26.0% (-11.5% to 65.0%) for the bug-based model; Eclipse improvement over the BUGS model was not significant. **Key null:** "inconsistent changes do not necessarily increase the number of future bugs". Risk sits in deviation from a function's or subsystem's usual pattern. The authors assume every code change requires a comment change, and flag that as a simplification. Human. [FT] — [Ibrahim 2012](https://sailresearch.github.io/sail-website/data/pdfs/JSS_OnTheRelationshipBetweenCommentUpdatePracticesAndSoftwareBugs.pdf)
- **Radmanesh et al., arXiv 2024.** 8 Apache Java repos (32 selected, cut for cost), GPT-3.5 used as the inconsistency labeller, odds ratios on bug-introducing commits. 7-day OR reported as 1.52, 14-day OR 1.14, earlier week 0.95. Per-repo 7-day OR from 0.72 to 2.68. **Caveats:** preprint leaves a placeholder for the sampled commit count; overall tables recompute to 1.42 (7-day) and 1.14 (14-day) and do not match the per-repo totals (arithmetic verified by me). Treat 1.5x as "small, possibly real". Association only. [HTML] — [Radmanesh 2024](https://arxiv.org/html/2409.10781v1)
- **Kaufman et al., arXiv 2026.** Controlled experiment, 86 Python programmers, 10 more in think-aloud; five comment conditions over correct and incorrect LLM-generated assertions. Accuracy 74% on correct assertions vs 49% on incorrect (OR 2.94, p<0.001), similar confidence. Explanations gave no overall benefit. Low-quality explanations reduced accuracy vs high-quality (OR 0.58, p=0.037) and raised confidence vs no comment (4.25 vs 3.99 of 5, p=0.005). Not stale comments specifically, but the closest controlled human evidence. LLM. [FT] — [Kaufman 2026](https://arxiv.org/pdf/2607.08885)
- **Abdelsalam et al., EMSE 2025.** Eye tracking, 20 CS students. Effect of comments on comprehension ranged from a 30% decrease to a 34% increase depending on snippet; comments took up to 23% of fixations; favourable perception did not consistently translate into performance. Comments were not manipulated to be false. Human. [ABS] — [Abdelsalam 2025](https://www.se.cs.uni-saarland.de/publications/docs/APB+25.pdf)
- **A Gothenburg reproduction study of comments and comprehension: page blocked, not opened.** Numbers not quoted.

#### LLMs
- **Xia et al., ASE 2026, "When Comments Mislead".** Most on-point. Abstract only: outdated comments create a systematic misleading effect rather than plain degradation, pull completions toward historical implementations, output looks plausible and fails where logic changed, effect varies with function signature length and task complexity. **No N, model, language or effect size available; ACM returned 403.** — [Xia 2026](https://conf.researchr.org/details/ase-2026/ase-2026-research-track/29/When-Comments-Mislead-An-Empirical-Study-of-Outdated-Comments-in-LLM-Based-Code-Comp)
- **CodeCrash, 2025.** 1,279 Python tasks (CruxEval, LiveCodeBench), 17 LLMs plus 3 reasoning models. Misleading comments: -24.3% Pass@1 relative, direct inference, combined (-21.3% Crux, -29.4% LCB). Overall across perturbations -23.2% direct, -13.8% with chain of thought. Per model on Crux/LCB: GPT-4o -14.0/-29.1, Claude-3.5-Sonnet -8.1/-9.6, Qwen2.5-7B -58.0/-38.3. "Ignore the comments" barely helps direct inference (GPT-4o 45.7 to 45.9); CoT recovers some (DeepSeek-V3 70.2 to 82.1, vanilla 92.1). **Comments are deliberately adversarial**, GPT-4o-written and filtered to be explicitly wrong, not realistic drift. Task is predicting execution output. [HTML] — [CodeCrash](https://arxiv.org/html/2504.14119v3)
- **Macke and Doyle, NAACL Findings 2024.** HumanEval, 164 functions, unit-test generation by GPT-3.5 and GPT-4. Docstring swapped for one from another function: 22.1% and 68.1% success, vs base 44.7% and 78.5%; significantly worse than every other condition (bootstrap, alpha 0.05). **Missing or partial documentation: no significant change.** "Incorrect" means random unrelated, not subtly stale. [FT] — [Macke 2024](https://www.arxiv.org/pdf/2404.03114)
- **Vitale et al., arXiv 2026.** Java, 11,637 test pairs, CodeT5+ and DeepSeek-Coder, bug fixing. Comments generated from the **fixed** code raise exact match from 2.78% to 9.80% (CodeT5+) and 5.09% to 12.77% (DeepSeek), trained with comments. This leaks the fix by design and the authors say the setup is inapplicable as-is in practice. Comments generated from the **buggy** code: same or slightly worse, 3.26% to 2.92% and 2.78% to 2.67%. Removing one misleading clause flipped a case to a correct fix (anecdote). [FT] — [Vitale 2026](https://arxiv.org/pdf/2601.23059)
- **Sabetto et al., 2025.** One IBM assembler module, 4 LLMs, MCQ quizzes of 10-37 questions. All comments replaced with an off-topic text: 73% to 61% on 10 shared questions. 20% of comments replaced: no meaningful change. Comment-free 84% vs commented 96% on the basic quiz. No statistical tests; the no-code baseline of 75% suggests training-data contamination. [HTML] — [Sabetto 2025](https://arxiv.org/html/2506.11007v1)
- **Null, adversarial comments in security review, 2026.** 100 synthetic vulnerable samples (50 Python, 30 JS, 20 Java), 8 models, 14,012 evaluations, McNemar on paired samples. No model changed significantly, all p>0.21: Claude Opus 4.6 95.0% to 94.0%, GPT-5.2 93.0% to 91.9%, DeepSeek Chat 89.0% to 93.0%. Single runs, keyword scoring, non-adaptive comments. Contrasts with the attack success rates it cites for generation contexts. [HTML] — [Comments vs reviewers](https://arxiv.org/html/2602.16741v1)
- **"Inside Out", 2025.** Activation steering on three Qwen-family models, about 375 records per concept. Deactivating comment concepts moved results from about -90% to +67% by task: helped translation, mostly hurt completion and refinement. Does not test false comments. [HTML] — [Inside Out](https://arxiv.org/html/2512.16790v1)

### Inferences
- Results across studies show an asymmetry worth a rule: wrong comments hurt, missing comments mostly do not (Macke), and correct comments can help (Vitale, though leaked). For an LLM reader, deletion is cheaper than falsehood. Evidence is 2024-2026, small, and on older models.
- Harm depends on task. Predicting behaviour and generating code are affected; detecting vulnerabilities with frontier models was not. Frontier models are the least-tested case in the harm literature.
- Instructing a model to ignore comments is weak mitigation in the one study that tested it, and only for direct inference.
- Ibrahim's pattern-deviation result is the only human-harm finding that points to a cheap check: flag changes that break a file's usual comment co-update habit. 2012, C and Java, unreplicated on agent repos.

### Gaps
- No controlled human experiment found that manipulates naturally stale comments and measures task performance.
- No controlled study on a frontier model with naturally stale comments and reported effect size. Xia 2026 may be it, but its numbers were unavailable.
- No study of an agent acting on stale comments across a real multi-file task.

## Q3. Accuracy of automated inconsistency detection, and practicality as review or CI

### Takeaway
Published scores do not transfer. A state-of-the-art tool fell from 0.83 to 0.08 precision on naturally occurring pairs, and plain prompting of GPT-4.1 and Llama 3.1-70B flags 82-97% of functions. The best natural-data figures are about 0.6 precision at unknown recall (DocPrism) or 0.88 precision at 0.21 recall on a balanced benchmark, near 0.4 in real use (CASCADE). None clears Google's 10% effective-false-positive bar with useful recall. Practical today as a periodic audit with a human or second-pass adjudicator, not as a blocking check.

### Cited Findings

| Study | Data, language | P | R | F1 | Notes |
|---|---|---|---|---|---|
| [Deep JIT, AAAI 2021](https://ar5iv.labs.arxiv.org/html/2010.01625) `[HTML]` | 40,688 synthetic pairs, 1,518 Java projects | 88.6 / 92.3 | 72.4 / 82.4 | 79.6 / 87.1 | full test (3,944) / cleaned (300). Not LLM |
| [CoCC 2024](https://arxiv.org/html/2403.00251v1) `[HTML]` | 22 Java projects; 5,000 Python | 92.1; 89.4 | 78.9; 75.7 | .85; .82 | Random Forest, author-reported. Method F1 .765, block .909 |
| [Radmanesh 2024](https://arxiv.org/html/2409.10781v1) `[HTML]` | CUP2 test, 385,258 (7,359 pos); verified 1,000. Java | 90.97; 94.4 | 96.38; 82.95 | 93.60; 88.30 | GPT-3.5 fine-tuned on 50 samples. Zero-shot 68.56/45.8/54.91, few-shot 75.80/52/61.68. Table 1 swaps R and F1. Unreplicated; treat sceptically |
| [C4RLLaMA, ICSE 2025](https://arxiv.org/pdf/2511.00215) `[VIA DocPrism]` | Panthaplackel reported vs natural Java | .94 reported; .83 [.67-.92] manual; **.08 [.05-.14] natural** | .87 binary labels | .88 | Flag rate 100% (500/500) on natural pairs |
| [DocPrism, ISSTA 2026](https://arxiv.org/pdf/2511.00215) `[FT]` | 1,991 natural pairs, 20 projects, 4 languages | .63 function (211 TP, 126 FP); .58 per finding | not measurable | .77 on 52-pair ablation only | Flag rate 17%. C++ precision .56. Kappa .737 on 200 |
| [CASCADE, FSE 2026](https://arxiv.org/html/2604.19400v1) `[HTML]` | 71 inconsistent, 814 consistent, 16 Java projects | .88 at 50/50 | .21 | .35; **.28 at 10/90** | Executes generated tests. 31 FP on 814 clean samples (3.8%) |
| [METAMON 2025](https://arxiv.org/html/2502.02794v1) `[HTML]` | 9,482 Defects4J test-doc pairs, Java | .722 | .480 | .576 | GPT-3.5, EvoSuite oracles vs Javadoc |
| [CCISolver 2025](https://arxiv.org/abs/2506.20558v1) `[ABS]` | CCIBench, derived from JITDATA | | | 89.54 | Deep-learning detector. Abstract: "a substantial portion of sampled data are mislabeled" |

- **Plain LLM prompting, natural data.** GPT-4.1: flag rate 82-91%. Llama 3.1-70B: 90-97%. Most flags are "under-promises", documentation being more concise than code. DocPrism's filtering cut flag rate from 98% to 14%, F1 from 0.22 to 0.77, precision from 0.14 to 0.71 on the ablation set. [FT] — [DocPrism](https://arxiv.org/pdf/2511.00215)
- **Plain LLM prompting, benchmark.** gpt-4.1-mini variants, 50/50: precision .53-.59, recall .29-.67, F1 .39-.59; at 10/90 F1 .16-.28. DocChecker: precision .48-.49, specificity .03-.19, close to always flagging. [HTML] — [CASCADE](https://arxiv.org/html/2604.19400v1)
- **Real-world CASCADE.** gpt-4o-mini, Java, C#, Rust. 13 reported to maintainers, 10 fixed. Table 4 as extracted gives 30 TP vs 44 FP, about 0.41 precision. The paper does not state this figure; my calc, and column mapping in the extraction was uncertain. Authors attribute the drop to the weaker model and low-quality documentation, and pitch it as an overnight post-hoc tool with human review. [HTML] — [CASCADE](https://arxiv.org/html/2604.19400v1)
- **Detectors fail on LLM-written comments.** Kang 2024: nine approaches including DocChecker, Deep-JIT and GPT-3 with and without CoT had no significant relationship with accuracy (Welch p from 0.168 to 0.747). Generating tests from the comment and running them did (point-biserial p<1e-11). ROC-AUC 0.63 on StarCoder, best 0.67. At threshold 0.8 it removes 46% of inaccurate comments and keeps 72% of correct ones. n=141 GPT-3 comments, Java. [HTML] — [Kang 2024](https://arxiv.org/html/2406.14836v1)
- **Label noise.** Panthaplackel found 17-20% noise in a human-reviewed sample: positive labels wrong 26-28%, negative 8-12%. The dataset pairs an old comment with new code, combinations that never existed in a repository, so scores on it are not scores on real staleness. [HTML] — [Panthaplackel 2021](https://ar5iv.labs.arxiv.org/html/2010.01625); [FT] [DocPrism](https://arxiv.org/pdf/2511.00215)
- **Narrow pattern tools.** Reported by DocPrism: iComment .61 on C/C++, @tComment .48 on Java, MPChecker and RustC4 above .95, restricted to particular patterns and languages. Fraco (rename fragility), per Radmanesh's table: .902 precision, .22 recall on the verified sample; .195 and .154 on the full set. Primaries not opened. [VIA DocPrism, Radmanesh] — [DocPrism](https://arxiv.org/pdf/2511.00215); [Radmanesh 2024](https://arxiv.org/html/2409.10781v1)
- **Review cost.** Labelling one DocPrism-flagged pair takes 5-15 minutes for a human. False positives are dominated by Incorrect Reasoning (38-61% by language) and Lack of API Knowledge (21-55%). [FT] — [DocPrism](https://arxiv.org/pdf/2511.00215)
- **Deployment bar.** Google: code-review checks must produce under 10% effective false positives, meaning developers take no positive action; Tricorder disables an analyser when "Not useful" clicks exceed 10%. Not comment-specific, measured practice. [FT] — [Sadowski 2018](https://6826.csail.mit.edu/2020/papers/google-analysis-cacm.pdf)

### Inferences
- With about 8% of blocks false, the reader's base rate is near CASCADE's 10/90 split. Quote F1 .28, not precision .88. 31 FP against about 15 TP on the same 885 samples implies precision around .33 (my calc), consistent with the ~.41 real-world figure.
- For a blocking CI check, nothing here meets the 10% bar. A scheduled audit whose flags get adjudicated matches how these tools were evaluated. Between 37% and 60% of flags are wrong, so an unadjudicated "fix what the detector flagged" loop would rewrite correct comments. Combine with Q4's rewrite error rate before automating.
- Execution-based checking (Kang, CASCADE) is the only family with a demonstrated relation to accuracy on LLM comments, and it can only reach behavioural claims that tests can express.
- Ask detection for incorrectness only. DocPrism shows that the framing, more than the model, moves flag rate from above 90% to below 20%.

### Gaps
- No independent replication on natural data except DocPrism's of C4RLLaMA.
- Frontier models (2026) are untested on natural staleness at measured recall. GPT-4.1 and gpt-4.1-mini are the newest evaluated here.
- No study deploying any of these as a CI step and reporting acceptance or effective false positives over time.
- No evaluation on repositories whose comments were LLM-written, apart from Kang's single-shot generation.

## Q4. How often an LLM introduces errors when it writes, rewrites, summarises or compresses a comment

### Takeaway
Generation: about a fifth of GPT-4 comments contain a false statement when strictly checked on long Java method comments; softer rubrics report "generally hallucination-free". Repair: about 56-65% correct by manual evaluation in the only two studies with human judgement. Compression: nothing measured. Summaries appear to follow names and surface cues more than code semantics.

### Cited Findings
- **Kang, Milliken, Yoo, 2024.** 540 comments on 180 Defects4J Java methods, StarCoder, GPT-3, GPT-4. At least half, a third, and a fifth inaccurate respectively. First author labelled all; 87% agreement on a subset. Taxonomy by severity: Hallucinating Intent, Hallucinating Reference, Lacking Code Context, Code Mischaracterization; the last two are more common, and the paper gives no category counts in text. [HTML] — [Kang 2024](https://arxiv.org/html/2406.14836v1)
- **Hallu-Eval, FSE 2026.** Qwen2.5-Coder-7B hallucination rate 17% on original snippets, 97% on semantically perturbed code. Hallu-Det F1 0.95 on that model's summaries. Hallu-Shield cut DeepSeek-Coder-6.7B from 66% to 59%, so mitigation is modest. Small models. [ABS] — [Hallu 2026](https://conf.researchr.org/details/fse-2026/fse-2026-research-papers/66/Hallucinations-in-LLM-based-Code-Summarization-Unveiling-Detection-and-Mitigation)
- **Liu, Lin, Thongtanunam, IJCNLP 2025.** About 50% of generated code reviews and about 20% of generated commit messages contain hallucinations. Common metrics are weak detectors alone; model confidence and feature attribution help most. [ABS] — [Liu 2025](https://arxiv.org/abs/2508.08661v1)
- **Repair.** C4RLLaMA, ICSE 2025: 65.0% correct in just-in-time and 55.9% post hoc, manual evaluation, CodeLLaMA fine-tuned. [ABS] — [C4RLLaMA](https://conf.researchr.org/details/icse-2025/icse-2025-research-track/10/Code-Comment-Inconsistency-Detection-and-Rectification-Using-a-Large-Language-Model). CCISolver, 2025: fixing success rate 0.6533, human evaluation. [ABS] — [CCISolver](https://arxiv.org/abs/2506.20558v1)
- **LLMCup, 2025.** Accuracy 49.0-116.9% better than CUP and HebCup; a user study found some updates better than human ones. Accuracy here is match to a reference, not truth, and no hallucination count is given. [ABS] — [LLMCup](https://arxiv.org/abs/2507.08671v1)
- **DocAgent, 2025.** Existence Ratio, share of mentioned repository entities that exist. DocAgent GPT-4o mini 95.74%, DocAgent CodeLlama 88.17%, plain chat GPT-4o mini 61.10%, plain chat CodeLlama 68.03%, fill-in-middle 45.04%. Checks existence, not truth. The Verified and Extracted counts in the table do not reproduce these ratios in the text I extracted (366 verified against 347 extracted, at 61.10%), so treat the figures as rough. [FT] — [DocAgent](https://arxiv.org/pdf/2504.08725)
- **Softer verdicts, which conflict with Kang's headline.** Rosen et al. 2024: MUMPS and IBM assembler, human raters, "generally hallucination-free, complete, readable, and useful", no automated metric correlates strongly; numbers not in abstract. [ABS] — [Rosen 2024](https://arxiv.org/abs/2411.14971v1). Vitale 2026: 400 sampled LLM comments, 93% coherent with their category, median adequacy 4 of 5. [FT] — [Vitale 2026](https://arxiv.org/pdf/2601.23059)

### Inferences
- The Kang and Rosen or Vitale gap is mostly a rubric gap: "any demonstrably false statement" vs a Likert adequacy score. A comment rule should use the strict rubric. Long comments contain more statements, so the strict rubric penalises them more, which ties back to the reader's result.
- An LLM rewrite of a stale comment has a measured failure of about a third to under a half on older fine-tuned models. Rewriting without a check can swap a known-suspect comment for an unknown-suspect one. Deleting the unverifiable claim avoids this, and Macke's missing-documentation null suggests the cost to an LLM reader is small.
- Hallu's 17% to 97% suggests summarisers follow surface signals. Name-heavy summaries should be treated as weak evidence about behaviour. The authors' framing is "logical hallucination"; the mechanism is my reading.

### Gaps
- **No measured study of LLM compression or shortening of comments**, or of rewriting a correct comment and whether its truth survives. This is the exact operation the question asks about.
- No model newer than GPT-4 class in a strict per-statement audit of generated comments.
- Generation error rate by length: not analysed. Kang's own sample design removes that variation.
- Searched for evidence that hallucination rises with output length: the FActScore paper was not opened, and an aggregator's claim about later facts being less accurate could not be confirmed. Not quoted.

## Q5. What LLM agents actually do to comments in real pull requests

### Takeaway
**No study found that measures source-comment density, additions, deletions or preservation in agent PRs against a human baseline.** AIDev studies measure documentation files or PR text and say so. Evidence on source comments is benchmark or vendor data, detector proxies, and one Python readability sample. Vendors disagree in direction. The strongest real-PR result is about PR text: claimed changes that were never made.

### Cited Findings

#### Source comments
- **Horikawa et al., MSR 2026.** AIDev, 1,382,314 agent commits; 4,115 readability-keyword commits (about 0.3%); 403 Python commits with metrics. Multi-line comments mean +2.10 median 0.00 (p<0.001, effect .63); comment lines mean +1.67 median 0.00 (p<0.001, .30); single-line mean +0.73 median 0.00 (p=.022, .17). LOC median +6.00 vs SLOC +2.60. 24.2% of agent commits address "Incomplete or Inadequate Code Documentation", vs 22.3% in a separate human dataset, not matched. Maintainability Index worsened in 56.1%. No human comment baseline, no deletion figures. [HTML] — [Horikawa 2026](https://arxiv.org/html/2603.13723v1)
- **Sonar, Aug 2025. VENDOR.** 4,442 Java tasks, SonarQube, six models. Comment density, defined as comment lines over total lines: Claude 3.7 Sonnet 16.4%, GPT-4o 4.4%. No other per-model value on the pages opened, no human baseline, no accuracy assessment. Benchmark tasks, not PRs. — [Sonar blog](https://www.sonarsource.com/blog/the-coding-personalities-of-leading-llms); [definition](https://www.sonarsource.com/the-coding-personalities-of-leading-llms)
- **Human baseline, 2009.** 5,229 active open-source projects: mean 18.67%, about 19%. Comment lines over total lines. Dedicated commenting about 2.5% of contributions. Small changes are denser: above 60% for one-line changes, approaching 19% for large, per Ibrahim's description. Wen reports highest density 25% for Java, a per-language value I did not find in the text. Human. [FT] — [Arafat 2009](https://dirkriehle.com/wp-content/uploads/2009/02/icse-2009-nier-for-web.pdf)
- **OX Security, Oct 2025. VENDOR.** "Comments everywhere (90-100%)": "AI models fill code with redundant comments", as markers for context limits. 300+ repos, 50 using AI tools. **What the percentage measures and its denominator are not stated; the mechanism is asserted.** Report PDF too large to fetch; read via Help Net Security. — [Help Net Security](https://www.helpnetsecurity.com/2025/10/27/ai-code-security-risks-report/)
- **Ghaleb, MSR 2026.** Classifies 33,580 AIDev PRs among five agents, 41 features. Claude Code described with "elevated comment density"; **19.8% is that feature's importance, not a density**, and is the only figure in the text. Per-agent values may be in a figure. Code-content features rank lower globally. n=458 for Claude Code. [HTML] — [Ghaleb 2026](https://arxiv.org/html/2601.17406v1)
- **Ji et al., arXiv 2026.** 8 repositories, 2021-2025, LLM detectors on comments, no ground truth. Flagged comment share stable over time, with detectors disagreeing widely (around 100% for three detectors in Zap, under 40% for all but one in Jadx). Flagged comments concentrate in Meta and Explanation categories (Go-github 97%), about 90% proper punctuation, low grammatical correctness. A detector-based proxy, as the authors stress, and flag rates that high are not credible as authorship. [FT] — [Ji 2026](https://arxiv.org/pdf/2607.01867)

#### Agent self-admitted debt
- **Brahmi et al., MSR 2026.** 525 agent-authored SATD comments from AIDev, 34 topics in 10 categories, mostly requirement and design debt; slightly more technically detailed than human SATD, both often without resolution guidance. Per-commit ratios are in the ACM PDF, which returned 403, so not quoted. [ABS] — [Brahmi 2026](https://2026.msrconf.org/details/msr-2026-mining-challenge/28/)
- **"TODO: Fix the Mess Gemini Created", TechDebt 2026.** Python and JS, Nov 2022-Jul 2025, 6,540 LLM-referencing comments, 81 debt comments after annotation (kappa .896). Design 40.74%, Requirement and Test 20.98% each, Defect 13.58%, Documentation 3.70%. Exploratory; human-written comments about LLM code. [HTML] — [Fix the Mess](https://arxiv.org/html/2601.07786v1)

#### Documentation files and PR text (not source comments)
- **De Michele and Tantithamthavorn, APSEC 2026 ERA.** 29,669 AIDev PRs, 2,585 repos, five agents. 63.2% change code with no documentation file touched (55.2% feature, 77.5% refactor; by agent 48.1% Claude Code to 72.8% Devin). **Inline comments and docstrings "count as drift", so the 63.2% is inflated for this purpose.** No human baseline: authors "do not claim this rate is specific to agents". Later fix 8.6-11.7% more often for undocumented files. Re-documentation within 90 days 24.0% vs 48.2%, adjusted OR 3.2 (95% CI 2.20-4.69). Codex is 65.8% of PRs, Claude Code 1.4%. [HTML] — [De Michele 2026](https://arxiv.org/html/2610.27381v1)
- **Yamasaki et al., MSR 2026.** 1,478 agent and 519 human documentation PRs; docs defined by file path. File co-editing: 66.1% agent-only, 30.2% human-only, 3.7% both. In 119 human-after-agent cases, 85.7% had agent additions at least equal to human deletions and 34.5% had zero human deletions; mean retention 86.8% where agent additions dominated. **No correctness assessment.** 29.0% of agent "docs PRs" touch no docs file. [HTML] — [Yamasaki 2026](https://arxiv.org/html/2601.20171)
- **Gong et al., MSR 2026.** 23,247 agent PRs, 974 annotated (kappa .892). High message-code inconsistency in 1.7% (406). Copilot 8.7%, Cursor 4.5%, Claude Code 0.9%, Codex 0.8%, Devin 0.4%. **Phantom Changes, descriptions claiming changes never implemented, 45.4%**; Scope Understated 22.0%; Placeholder or Incomplete 18.8%. Acceptance 28.3% vs 80.0%; merge time 55.8 h vs 16.0 h. Heuristic detector, precision .742, recall .548, so 1.7% is likely low. No human baseline. [HTML] — [Gong 2026](https://arxiv.org/html/2601.04886v2)

#### Checked, no comment data
- [Ogenrwot and Businge, MSR 2026](https://arxiv.org/html/2601.17581v3): 24,014 agent and 5,081 human PRs; description-diff similarity only. [HTML]
- [Mazloomzadeh et al., 2026](https://arxiv.org/html/2607.21832v1): 9,428 agent PRs, Python; comments grouped under a Documentation topic, first 100,000 characters read. [HTML]
- [GitClear 2025, VENDOR](https://www.gitclear.com/ai_assistant_code_quality_2025_research): 211M changed lines, landing page says nothing on comments; PDF not opened.
- [CodeRabbit Dec 2025, VENDOR](https://coderabbit.ai/blog/state-of-ai-vs-human-code-generation-report): 470 PRs (320 AI co-authored, 150 assumed human; the authors "cannot guarantee" those labels). Readability issues over 3x, naming nearly 2x, formatting 2.66x. No comment or documentation numbers. Full report behind a form; not opened.
- MSR 2026 challenge programme titles scanned for the first 100,000 characters of the page: no title names code comments or docstrings. [Programme](https://2026.msrconf.org/track/msr-2026-mining-challenge)

#### Adjacent, student comments steering Copilot
- Giacaman et al., SIGCSE Virtual 2026: 67,787 comments, 1,161 students. What 77.0%, How 21.0%, Why 0.4%. Human-written prompts, not LLM comments. [FT] — [Giacaman 2026](https://arxiv.org/pdf/2607.10674)

### Inferences
- "LLMs over-comment" is unproven. Sonar's 4.4-16.4% sits at or below the human ~19%, OX says the opposite without a denominator. Density in short benchmark tasks and in whole repositories are not comparable, so neither settles it. Per-model variation (almost 4x in Sonar) is the more reliable fact.
- The nearest real-PR analogue to false comments is Phantom Changes: agent text asserting work not done, 45% of high-inconsistency cases, with reviewers rejecting them. Whether agents make the same claims in code comments is untested. It is a plausible risk for change-narration comments.
- Yamasaki's retention suggests agent text often survives without human revision. In an agent-maintained repository nobody plays that human, which removes the check that was already weak.
- The reader's own repository may be one of the better available datasets for this question.

### Gaps
- Agent vs human comment density at PR level, comment deletion or preservation rates, and narration comments ("now uses X", "changed from Y"): no measurement found. AIDev has the diffs, so the study is feasible and apparently undone in what I could search.
- Mazloomzadeh's RQ3 "Number of Comments" results were not read; the first 100,000 characters did not establish whether it counts review or source comments.
- Remaining MSR 2026 challenge titles past the first 100,000 characters were not scanned.
- `How do Agents Refactor` (MSR 2026) was not opened; it may report comment handling.

## Q6. Mechanical checks with evidence

### Takeaway
No mechanical comment check found has a measured effect on staleness. Tools exist and work as described, but their documentation contains no accuracy or outcome data. Evidence exists for the problem they target (link rot, outdated references, TODO removal), not for the intervention. Combined with DocPrism's error mix, signature and symbol checks address about a fifth of confirmed false statements.

### Cited Findings

#### Symbol, signature and reference checkers
- **rustdoc lints.** `broken_intra_doc_links` warns by default on unresolved or ambiguous links; `invalid_rust_codeblocks` warn; `missing_doc_code_examples` allow, nightly-only. Can be promoted to error with `#![deny(...)]`. Tool documentation, no evaluation. — [rustdoc lints](https://doc.rust-lang.org/rustdoc/lints.html)
- **clang `-Wdocumentation`.** "Emit warnings about use of documentation comments. This warning group is off by default." I verified that sentence only, not its exact checks. — [Clang manual](https://clang.llvm.org/docs/UsersManual.html)
- **pydoclint.** Checks arguments, returns, yields, raises, class attributes, and type hints against docstrings; 39 rules. Skips short docstrings by default. Claims 3 s on numpy, no hardware stated. No accuracy evaluation. — [pydoclint](https://github.com/jsh9/pydoclint)
- **DOCER.** Regex extraction of code references checked against two source revisions. Agreement kappa .92 on 50 elements. 19 instances reported across 15 Google projects: 5 fixed; 4 projects responded positively, 4 reported false positives, 7 no response. No precision or recall reported; README and wiki only. A later ICSME 2023 GitHub Action tool paper exists, not opened. [HTML] — [DOCER](https://ar5iv.labs.arxiv.org/html/2212.01479)
- **Link checking, Hata 2019.** 9 PRs fixing dead links in comments, all merged per Section III-G (abstract says most). Evidence that such fixes are welcomed, not that they prevent rot. [HTML] — [Hata 2019](https://ar5iv.labs.arxiv.org/html/1901.07440)

#### Executable documentation
- **Go testable examples.** Compiled as part of tests, executed when an `Output:` comment is present, output compared. The Go blog's claim that this "guarantees that the information will not go out of date as the API changes" is authors' opinion about a mechanism; no measurement. — [Go blog](https://go.dev/blog/examples)
- **Doctest in general.** One search found no empirical staleness or breakage rate, only tool docs and anecdotes. Thin; a targeted search could still find something.
- **Document testing, closest measured.** Kang's and CASCADE's LLM-generated tests from comments: numbers in Q3. Generated, not maintained, tests.

#### TODO-with-expiry
- **`unicorn/expiring-todo-comments`.** Date, package, engine and dependency conditions. `checkDates` off by default; by default dates are skipped on PRs, and TODOs with no condition are ignored. No evaluation. — [rule docs](https://github.com/sindresorhus/eslint-plugin-unicorn/blob/main/docs/rules/expiring-todo-comments.md)
- **`todo_or_die` (Ruby).** Code, not comment; raises on overdue except in Rails production, where it logs. README warns it can break production. No outcome data; cites "LeBlanc's Law" anecdotally. — [todo_or_die](https://github.com/searls/todo_or_die)
- **TDCleaner, FSE 2021.** Learned model on top-10,000 Python and Java repos. 18 obsolete TODOs reported, 9 confirmed and removed. Precision not on the abstract page. [ABS] — [TDCleaner](https://arxiv.org/abs/2108.05846v1)
- **On-hold SATD, Maipradit et al.** 335 comments studied; classifier AUC .83 for identifying comments waiting on an external condition. [ABS] — [Maipradit](https://arxiv.org/abs/1901.09511)
- **TrigIt (Nie et al.):** searched, not found in results, not opened.

#### Ratchets
- **No controlled evidence found.** One search returned practitioner writeups (Notion's ESLint ratchet, an Apple LLVM talk) that I did not open. No study measuring whether ratchets reduce warnings or improve quality, and none about comments.
- **A warning about what a ratchet would count.** Zampetti: 20-50% of SATD comments vanish because the surrounding code was deleted, so a comment count falling is not debt falling. [ABS] — [Zampetti 2018](https://aserebre.win.tue.nl/MSR2018-Fiorella.pdf)

### Inferences
- By DocPrism's mix, mechanical name and signature checks can reach about 19% of confirmed false statements (Parameter 7, Return 5, Identifier 4, Type 3), 22% with unused parameters. In C++ alone about 14%, or 29% with unused parameters. Behavioural claims, 77% overall, are out of reach. Worth having because they are cheap and precise, but they cannot stand in for review.
- Wen's low-update categories (Name, Literal, Method Invocation) are where a mechanical check has the highest yield: a rename or literal change is detectable, and those are the changes developers most often leave uncommented. Fraco's .90 precision at .22 recall is the shape to expect.
- C++ has no language-level doc-test mechanism, unlike Go and Rust, so the equivalent is a comment that cites a test by name, with a script checking the name resolves. This checks existence, not truth, per DocAgent's Existence Ratio. The repository's citation check in `tools/gate_extra.sh`, named in `AGENTS.md`, may already occupy this slot; I did not read it.
- Given the evidence, the defensible rule set is: prefer mechanisms with no truth claim to go stale (delete, shorten, move behaviour into a test); check names mechanically; audit behavioural claims periodically with adjudication; do not gate merges on an LLM detector. This is my synthesis, not a measured intervention.
- The reader's study is itself a measurement instrument: re-running it after a rule change would produce the outcome data this literature lacks, provided the count is per claim.

### Gaps
- No before/after study of any comment linter, doctest requirement, TODO expiry or ratchet on staleness rate.
- `LINT.IfChange`-style co-change markers: not researched.
- Evidence on checks in agent-maintained repositories: none found.
