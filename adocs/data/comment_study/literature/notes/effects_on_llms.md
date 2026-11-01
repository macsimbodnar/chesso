# Measured effects of source-code comments on LLMs doing software-engineering tasks

As of 2026-11-01. "Points" = absolute percentage points. `[pre-2024]` = model older than 2024, may not transfer. No study here tested a model newer than Claude Opus 4.7 / GPT-5.4-class, and Opus 4.7 appears only as a comment *source*.

Read status. **PDF** = arXiv PDF text extracted, numbers checked against table text by me. **HTML** = read only through the fetch tool's small-model extractor, not checked. That extractor made 3 errors on papers I then checked (flip count 3 reported as 33; RepoQA 13/15 reported as 14/15; Thornton C1-C3 reported as absent), so treat HTML-only numbers as less safe. **Abstract only** = paper not opened, numbers not quoted.

## Do comments help or hurt in comprehension, generation, bug finding, repair and agentic tasks?

### Takeaway
Truthful comments move modern models by small amounts, and the sign depends on task and model. The large gains all come from comments that carry information the code lacks: oracle comments written from the correct solution, or a docstring that is the only specification. One repo-level agent experiment exists; it is a blog post with no published numbers.

### Cited Findings

**Study cards**

| ID | Year, venue | Design | N | Models | Lang | Read |
|---|---|---|---|---|---|---|
| [Macke](https://arxiv.org/abs/2404.03114) | 2024 preprint | Unit-test generation as comprehension proxy; bootstrap α=0.05 | 164 HumanEval functions | gpt-3.5-turbo, GPT-4 `[pre-2024]` | Python | PDF |
| [Sabetto](https://arxiv.org/html/2506.11007v1) | 2025 preprint | MCQ quizzes by 2 SMEs; 3 runs per cell; no test, no variance | 1 module (L8WAIT); 20, 37, 37, 20 questions | Claude-3, Llama 3, Mixtral, GPT-4 | IBM assembler | PDF |
| [Pan26](https://arxiv.org/abs/2609.09242) | 2026 preprint | Prefill comment block into recipient's own output; paired exact McNemar, Holm | 1055 LCB problems; RepoClassBench C# 61, Java 130 | Recipients: Gemma-4-E4B-IT, Qwen3-8B, Seed-Coder-8B, CodeGeeX4-9B. Sources: Grok-4.1-Fast, Gemini-3.1-Flash-Lite, Claude-Opus-4.7 | Python, C#, Java | PDF |
| [Imani](https://arxiv.org/abs/2512.16790) | ICSE 2026 | Paired removal plus embedding steering; single run, temp 0; no significance tests | Completion 1046, refinement 103, translation 47 sampled (Table 1 layout ambiguous in extraction; translation percentages imply n=43) | Qwen2.5-Coder-32B, Qwen2.5-32B, QwQ-32B, all 4-bit | Java | PDF |
| [Vitale](https://arxiv.org/abs/2601.23059) | 2026; ICPC 2026 per [repository file name](https://iris.polito.it/retrieve/5d3a3db1-dfec-411e-8212-1645451c21ac/icpc_vitale_26.pdf) | Fine-tune with/without comments, test with/without; McNemar, OR | 116,372 pairs; 11,637 test | CodeT5+, DeepSeek-Coder 1.3B `[pre-2024]`; GPT-4.1 zero-shot as a check | Java | PDF |
| [Antimemetics](https://antimemetics.institute/comment-ablation) | Feb 2026 blog, not peer reviewed | mini-swe-agent on comment-stripped SWE-bench Verified; 3 runs per configuration | N of tasks not stated | GPT-5-mini, GPT-5.2, 4 reasoning levels each | Python (SWE-bench) | Web page |
| [RepoQA](https://arxiv.org/abs/2406.06025) | 2024 preprint | Find a function from its description in 16k-token context; no test | 500 tasks, 50 repos | 15 models with comment-free column | Py, C++, Rust, Java, TS | PDF |
| [Dainese](https://aclanthology.org/2024.eacl-srw.24.pdf) | EACL SRW 2024 | HumanEval pass@1 under docstring variants | 164 problems | OpenLLaMA, MPT-7B, StarCoder, WizardCoder x3 `[pre-2024]` | Python | PDF |
| [RealClassEval](https://arxiv.org/abs/2510.26130) | 2025 preprint | Paired docstring ablation, Friedman, Wilcoxon, FDR, Cliff's δ; power 0.78-0.87 | 400 real classes, 100 ClassEval | Qwen 2.5 Coder, GPT-4.1, GPT-5, GPT-OSS, Codestral, DeepSeek-V3, Llama-4 Maverick | Python | PDF |
| [CodeCrash](https://arxiv.org/abs/2504.14119) | [NeurIPS 2025](https://proceedings.neurips.cc/paper_files/paper/2025/file/aec5e2847c5ae90f939ab786774856cc-Paper-Conference.pdf) | Output prediction under injected false text; N=3 direct, N=1 CoT; no test seen | 1279 questions | 17 LLMs plus o3-mini, DeepSeek-R1, QwQ | Python | PDF |
| [Haroon](https://arxiv.org/abs/2504.04372) | Jan 2026 preprint | Keep faults the model localized, mutate, re-ask | 750,013 tasks from 1,307 programs | GPT-4o, Claude 3.7 and 4.5 Sonnet, Gemini x3, 4 open | Python, Java | PDF for quoted lines |
| [Thornton](https://arxiv.org/abs/2602.16741) | Feb 2026 preprint, titled "Can Adversarial Code Comments Fool AI Security Reviewers" in PDF and "LLM Code Reviewers Are Harder to Fool Than You Think" in HTML v1 | Vulnerability detection; McNemar on C0 vs C4 | 100 synthetic samples of 15-60 lines; 14,012 evaluations | Opus 4.6, GPT-5.2, Gemini 2.5 Pro, DeepSeek Chat, Sonar Pro, 3 open | Py, JS, Java | PDF |
| [ALIBI](https://arxiv.org/abs/2607.24964) | Jul 2026 preprint | Adaptive attacker writes comments, 5 refinement rounds | 125 CWE-476, 70 CWE-416 | OpenVul, VulnLLM-R, Vul-RAG (GPT-4o-mini), VulTrial (GPT-4o) | C/C++ | PDF for main tables, HTML for static baseline |
| [Li](https://arxiv.org/abs/2512.16538) | Dec 2025 preprint | 19 obfuscations incl. comment deletion (L2), done by GPT-4o | Count 128, 143, 84, 210 per dataset | 15 LLMs incl. gpt-4o, o3-mini, DeepSeek-R1 | Solidity, C, C++, Python | PDF, one table |
| [Xia](https://conf.researchr.org/details/ase-2026/ase-2026-research-track/29/When-Comments-Mislead-An-Empirical-Study-of-Outdated-Comments-in-LLM-Based-Code-Comp) | ASE 2026 | Outdated comments in completion | Not available | Not available | Not available | **Abstract only.** ACM returned 403, no preprint found |
| [Johnson](https://www.frontiersin.org/journals/artificial-intelligence/articles/10.3389/frai.2026.1784973/full) | Frontiers AI 2026 | Self-labelled pilot; 7 raters score outputs; Krippendorff α 0.23 | 1 algorithm, 5 comment styles | Claude Opus 4.1, GPT-5 | Python | HTML |

**Comprehension**
- With all comments, quiz scores averaged 96% (basic) and 90% (advanced), against 84% on both with none. Llama 3 went 80 to 100 and Mixtral 60 to 85 on the basic quiz; GPT-4 "had little variance". A no-code condition still scored 75%, so the module was probably already known to the models. No test, one module. — [Sabetto](https://arxiv.org/html/2506.11007v1)
- Comments vs none: no significant change in failed tests (bootstrap), but significantly higher line coverage for both models. Base-file success was 44.7% (GPT-3.5) and 78.5% (GPT-4). `[pre-2024]` — [Macke](https://arxiv.org/abs/2404.03114)
- **Removing comments raised retrieval accuracy for 13 of 15 models.** With comments vs comment-free: gpt-4o-2024-05-13 90.6 vs 93.2; gpt-4-turbo 76.4 vs 92.6; deepseek-coder-33b 48.4 vs 75.4; Llama-3-70B 82.2 vs 86.2. Both Gemini models fell: 1.5-pro 90.6 vs 90.2, 1.5-flash 90.0 vs 54.2. The flash drop is an artifact the authors traced to padding: the context was padded back to 16k with `# {LINE_NUMBER}` comments and the model "simply continues counting". No test. — [RepoQA](https://arxiv.org/abs/2406.06025)
- Java to Python translation, with vs without comments: Code LLM 51.16 vs 46.51; Generic 39.53 vs 41.86; Reasoning 76.74 vs 83.72. These percentages equal 22/43 vs 20/43, 17/43 vs 18/43 and 33/43 vs 36/43 (my derivation), so the spread is 1 to 3 samples. — [Imani](https://arxiv.org/abs/2512.16790)

**Generation and completion**
- Real classes, full docstrings vs signatures only: differences of 0.44 to 3.13 points favouring docstrings, **0 of 28 comparisons significant after FDR**, Cliff's δ 0.003 to 0.077 (negligible). GPT-5: 1.59 pre-cutoff, 0.93 post-cutoff. Authors: "genuinely minimal rather than obscured by insufficient sample sizes". — [RealClassEval](https://arxiv.org/abs/2510.26130)
- HumanEval, docstring removed: average pass@1 34.9 to 14.8; WizardCoder-15B 57.9 to 20.1; StarCoder 33.5 to 15.9. Here the docstring is the whole specification. `[pre-2024]` — [Dainese](https://aclanthology.org/2024.eacl-srw.24.pdf)
- Repo-level completion (CrossCodeEval), EM with vs without comments, Code Match: Code LLM 13.88 vs 12.81; Generic 18.15 vs 16.01; Reasoning 13.88 vs 11.39. ID Match: 32.38 vs 30.96; 31.32 vs 29.18; 27.76 vs 23.49. No test. — [Imani](https://arxiv.org/abs/2512.16790)
- Comments the model writes itself: among 16 models with pass@1 ≥ 80%, comment share of output spans 0.1% to 71.4%. Within-model correlation with pass@1 is at most |ρ| 0.32 on form and 0.53 on token share. Volume does not predict success. — [Pan26](https://arxiv.org/abs/2609.09242)
- **Telling a model not to comment hurt some models.** Prompt "none": Gemma-4-E4B-IT 53.7 to 39.2 (−14.5, p_holm 2.3e-30); GPT-5.4-Nano −13.6; Gemini-3.1-Flash-Lite −6.9. Authors: models with a heavy head-block habit are "the most damaged by being told not to". Mistral-Small lost 43% of code tokens with no significant change. — [Pan26](https://arxiv.org/abs/2609.09242)

**Bug and vulnerability finding**
- Detection rate by comment variant, read from the Figure heat map (C0 none / C1 accurate / C3 misleading): Opus 4.6 95.0 / 96.0 / 94.0; GPT-5.2 93.0 / 94.0 / 92.0; Gemini 2.5 Pro 91.9 / 93.0 / 91.0; DeepSeek Chat 89.0 / 92.0 / 87.0; Sonar Pro 96.1 / 96.0 / 92.0. C1 here includes "explicit vulnerability warnings", so it is close to being told the answer and still moved 0 to 3 points. I saw no test on C1 or C3. — [Thornton](https://arxiv.org/abs/2602.16741)
- Comment deletion vs original, detection rate on Smart-bugs / ReposVul cpp / ReposVul py / PrimeVul: gpt-4o 0.68→0.70, 0.26→0.25, 0.27→0.25, 0.16→0.11; o3-mini 0.89→0.82, 0.26→0.30, 0.14→0.15, 0.27→0.31; DeepSeek-V3 0.83→0.88, 0.29→0.32, 0.24→0.29, 0.17→0.22; DeepSeek-R1 0.78→0.79, 0.24→0.20, 0.20→0.23, 0.29→0.35. Authors note "modest yet consistent improvements" on ReposVul py. Mixed sign; original comment density not reported in what I read. — [Li](https://arxiv.org/abs/2512.16538)
- **Contradicting its own headline.** Stripping comments vs adversarial comments: Opus 94.0 to 96.9; GPT-5.2 91.9 to 95.9; DeepSeek 93.0 to 89.0; Sonar 92.0 to 91.0; 30% (17/57) of misses recovered. The paper says stripping "actually hurts" weaker models. That rests on DeepSeek returning to its C0 of 89.0, after a C4 gain with p=0.219. — [Thornton](https://arxiv.org/abs/2602.16741)

**Program repair**
- EM on test without vs with comments. CodeT5+ trained with comments 2.78 to 9.80 (OR 6.86); DeepSeek-Coder trained with comments 5.09 to 12.77 (OR 9.76). Trained without comments: CodeT5+ 3.26 to 3.73 (OR 1.44); DeepSeek 5.57 to 7.90 (OR 3.42). GPT-4.1 zero-shot 6.11 to 8.63. `[pre-2024 for the two fine-tuned models]` — [Vitale](https://arxiv.org/abs/2601.23059)
- **That gain is oracle-fed.** Comments were generated by GPT-3.5 "for the fixed version" and prepended to the buggy method. Comments generated from the buggy code gave "same or slightly worse": CodeT5+ 3.26 to 2.92 and 2.78 to 2.67. Dataset 2011-2017. — [Vitale](https://arxiv.org/abs/2601.23059)

**Multi-file agentic**
- GPT-5-mini: "small but statistically significant increase in pass rate upon comment removal", at every reasoning level. GPT-5.2: "a trend toward comments being more helpful, but no significant effect". No pass rates, N, test name or p-value on the page. Test resources asserting comments, type hints, shebangs and lint directives were kept; docstring treatment not stated. — [Antimemetics](https://antimemetics.institute/comment-ablation)

### Inferences
- The signal is new information, not comment presence. Oracle comments (Vitale, Pan26, Dainese's oracle hints 34.9 to 42.6) produce the gains; docstrings on real classes where the signature and context already suffice give δ under 0.08.
- Weak models gain and strong models do not: Sabetto's GPT-4 flat vs Mixtral +25; Pan26 tested only weak recipients. Do not transfer Pan26's +17.2 to a frontier agent.
- Thornton gives a noise yardstick. Stripping C4 should reproduce near-C0 code, yet stripped vs C0 reads Opus 96.9 vs 95.0, GPT-5.2 95.9 vs 93.0, DeepSeek 89.0 vs 89.0, Sonar 91.0 vs 96.1. So gaps of 2 to 5 points are plausibly noise there, which covers every C1 and C3 cell, assuming the stripped text does match C0.
- Imani's "-90% to +67%" is not a comment-removal effect. It is embedding steering. The +66.67% is 3/12 to 5/12 (my derivation). The −88.56% BLEU sits beside BLEU-trim 77.81 to 77.93, so it is an output-format artifact, not lost quality.

### Gaps
- No peer-reviewed comment ablation on a frontier model doing repo-level tasks. Two targeted searches found no second repo-level comment ablation on agents besides Antimemetics.
- No study on C++ specifically, or on a performance-critical codebase. Closest are C/C++ security sets.
- Not opened: [Agent-Diff](https://arxiv.org/pdf/2602.11224), surfaced only as a search snippet saying irrelevant documentation diluted benefit.

## What happens when comments are wrong, stale or misleading? Is a false comment worse than none?

### Takeaway
False content is usually about as bad as no comment or somewhat worse. Off-topic or explicitly contradictory text measures worse than none: −24% relative on average on direct inference, −52% at worst for one model, and about −3 to −12% for reasoning models. A small fraction of false comments among true ones showed no effect. Plausible false claims about a result are the worst measured class. Real staleness has one dedicated study, and I could not read its numbers.

### Cited Findings

**False vs none, direct comparisons**
- Docstring from a different function vs no comment: success 44.7 to 22.1 (GPT-3.5), 78.5 to 68.1 (GPT-4), significant vs every other condition; coverage also significantly lower. **The cleanest "false is worse than none" result, on simple functions.** `[pre-2024]` — [Macke](https://arxiv.org/abs/2404.03114)
- Prefill, on correct-solution problems. Correct block vs corrupted block with one key step wrong (about 2.3 of 10 lines): Qwen3-8B 70.8 vs 48.2 (baseline 53.6); Seed-Coder 58.3 vs 41.9 (43.9); CodeGeeX4 52.3 vs 28.4 (26.4); Gemma 85.2 vs 74.2 (71.3). Corrupted differs from baseline significantly only for Qwen3-8B (−5.4). **One wrong step erases the lift and lands near no comment.** — [Pan26](https://arxiv.org/abs/2609.09242)
- Wrong-topic block: −20.8 points over 12 cells; random text −17.9, "coherent but topic-wrong comments hurt more than incoherent noise", significant in a paired test. Damage concentrates on easy problems the model would have solved alone. Source-wrong: none of 12 cells significant, with a floor effect the authors flag. — [Pan26](https://arxiv.org/abs/2609.09242)
- 20% of comments replaced with unrelated restaurant-tip comments: "negligible"; Claude-3, GPT-4 and Llama 3 scored identically to no comments. All comments replaced with an NBA description: 73 to 61 on 10 questions, against accurate comments rather than none. GPT-4 then described the code as "a comprehensive simulation tool for predicting the outcome of an NBA season". — [Sabetto](https://arxiv.org/html/2506.11007v1)
- Comments generated from buggy code: same or slightly worse than none (CodeT5+ 3.26 to 2.92). Closest analogue to a comment that documents a bug. — [Vitale](https://arxiv.org/abs/2601.23059)
- Subtly false security comments (C3) vs none: −1.0, −1.0, −0.9, −2.0, −4.1 points on the five commercial models. Adversarial comments (C4): no model significant, all p > 0.21; Opus 4.6 95.0 to 94.0, p=1.000. A skeptical system prompt treating comments as untrusted recovered 43% of C4 misses (HTML). — [Thornton](https://arxiv.org/abs/2602.16741)

**False text injected, relative change in pass@1 on output prediction**

| Model | MCC Crux | MCC LCB | MHC Crux | MHC LCB |
|---|---|---|---|---|
| Claude-3.5-Sonnet | −8.1% | −9.6% | −14.4% | −43.4% |
| GPT-4o | −14.0% | −29.1% | −6.4% | −26.6% |
| Gemini-2.0-Flash (derived in HTML, not checked) | −26.8% | −51.9% | −8.3% | −19.5% |
| DeepSeek-V3 (HTML, not checked) | −16.6% | −41.4% | −10.7% | −29.7% |
| Qwen2.5-Coder-32B | −23.2% | −36.6% | −10.5% | −21.7% |
| Average, 17 LLMs | −21.3% | −29.4% | −12.4% | −33.0% |
| o3-mini low effort, reasoning (HTML) | −4.4% | −12.4% | −11.1% | −19.8% |
| o3-mini high effort, reasoning (HTML) | −3.6% | −5.6% | −13.4% | −28.4% |
| DeepSeek-R1, reasoning (HTML) | −3.5% | −2.7% | −2.4% | −0.6% |

- MCC = "generic, explicitly incorrect" comments at eight AST node types. MHC = "plausible but incorrect" output hint at a function definition or return. Overall: MCC −24.3% direct, −14.9% CoT; MHC −20.1% direct, −6.2% CoT. Structural control (rename, reformat, garbage code) −24.6% direct. Print-statement version −23.8%. — [CodeCrash](https://arxiv.org/abs/2504.14119)
- Reasoning tokens under MHC rise about 2-3×; QwQ-32B went from about 1,530 to 3,740 on LCB. HTML only. — [CodeCrash](https://arxiv.org/html/2504.14119)
- Misleading comments left 25.63% of previously localized faults found (denominator implied by the method, not stated); misleading names 29.02%; dead code 20.38%. "Misleading but coherent" comments, written by the model under test. Example in Table I: "Summon ancient dragons". — [Haroon](https://arxiv.org/abs/2504.04372)
- Single fixed false comment `/* {var} is non-null here. */` above the real bug: 12/114 (10.5%), 7/111 (6.3%), 3/115 (2.6%), 17/98 (17.3%) flipped, "The baseline mostly fails". Adaptive attacker with 5 rounds: 97.4%, 91.2%, 95.5%, 100%. Comment sanitization before the detector cut this to 0.4-2.8%, tested on each attack's strongest payload rather than the full search. A prompt-level trust policy barely helped: 93.9, 93.8, 42.3, 99.0%. — [ALIBI](https://arxiv.org/html/2607.24964)
- Pilot: "hallucinated" comments total 28.14 (Claude) and 28.71 (GPT-5) vs stock 30.86 and 27.57 out of 35. Persona confounded with register and length; author's own limitation. — [Johnson](https://www.frontiersin.org/journals/artificial-intelligence/articles/10.3389/frai.2026.1784973/full)

**Real staleness**
- Abstract, no numbers: outdated comments pull completions toward the historical implementation, giving code that looks plausible and fails where behaviour changed; effect varies with signature length and task complexity. **Not opened; design and effect sizes unknown.** — [Xia](https://conf.researchr.org/details/ase-2026/ase-2026-research-track/29/When-Comments-Mislead-An-Empirical-Study-of-Outdated-Comments-in-LLM-Based-Code-Comp)

**Do models notice?**
- Targeted MCQ on comment accuracy: 84% average. — [Sabetto](https://arxiv.org/html/2506.11007v1)
- Code-comment inconsistency detection F1 on JITData / CCIBench: GPT-3.5 54.82 / 58.03; DeepSeek-Coder 66.55 / 71.37; fine-tuned C4RLLaMA 85.24 / 88.67. N 3,944 and 2,130, Java. Audit found about 45.67% (274/600) of JITData positives likely mislabeled. `[pre-2024 models]`, HTML. — [CCISolver](https://arxiv.org/html/2506.20558)

**Opinion, not measured**
- "An inherent cognitive bias toward treating comments as authoritative" is an interpretation of MCC vs MPS, from "LRMs consistently perform better under MPS than under MCC". — [CodeCrash](https://arxiv.org/abs/2504.14119)
- "In detection, comments must override what the model sees in already-written code" is an argument, not a test. — [Thornton](https://arxiv.org/html/2602.16741v1)

### Inferences
- Yes, false can be worse than none, but it depends on the falsehood. Off-topic or contradictory: worse. A wrong detail inside an otherwise right comment: near none, and it destroys the comment's value. A minority of false comments among true ones: no effect in the one test, on one module.
- Most relevant to stale comments: Pan26's corrupted condition, Vitale's buggy-code comments, Thornton C3, MHC, and Xia. Most adversarial numbers (ALIBI adaptive, CodeCrash MCC density) overstate what honest staleness does. ALIBI's static baseline of 2.6-17.3% is the closer figure.
- Plausible output claims are more dangerous than obvious falsehoods for reasoning models: o3-mini high effort −5.6% MCC vs −28.4% MHC on LCB. A confident "returns X" that is wrong is the case to prevent.
- CodeCrash and Haroon show large falls from any perturbation. Structural noise costs as much as false comments, so comment falsity is not shown to be special in those two.
- Haroon's 25.63% has no neutral control. It conditions on earlier success, so regression to the mean is included; a search of the text for re-run or temperature controls found none. It likely overstates harm.

### Gaps
- No direct test of "model follows comment vs follows code" at the decision level, on current models.
- Xia's numbers. Worth buying or requesting: it is the only dedicated study of real staleness.
- No measure of how often a stale comment misleads an agent that can also run tests.
- 66.9% synthesis success under inaccurate docstrings appeared only in a search-result summary. I could not find the source, so it is not used.

## Which kinds of comment carry the effect: why/intent, contract/docstring, what-restating, inline, examples?

### Takeaway
No controlled study has compared why vs what-restating comments on repository tasks. The strongest result is that specificity carries the effect: vague rewrites hurt, while shorter, paraphrased or hedged ones do not. Evidence that why beats what is suggestive and fails correction.

### Cited Findings
- **Style rewrites of correct blocks, change vs source-correct (Qwen3-8B / Seed-Coder-8B):** paraphrased −1.0 / −1.0; compressed to 40-60% of characters −2.7 / +3.4 (p<0.05); hedged −0.3 / +1.4; vague −7.0 (p<0.001) / −5.3 (p<0.01). n = 618-724 per condition. Only "removing specific task information hurts". — [Pan26](https://arxiv.org/abs/2609.09242)
- Line-shuffled correct blocks keep 68.0% of the lift (+11.7 vs +17.2); 10 of 12 cells significant. Authors read this as lines being independent local hints, not a derivation. — [Pan26](https://arxiv.org/abs/2609.09242)
- Intent, observational across 13 models: WHY ρ = +0.61, raw p = 0.027, fails Bonferroni α 0.0167. WHAT ρ = −0.20, p = 0.52. Judge κ: WHY 0.74, WHAT 0.36. Authors treat the null as low power. — [Pan26](https://arxiv.org/abs/2609.09242)
- Contradicting: told to write WHAT comments, GPT-5.1-Codex-Mini went 64.6 to 69.5 (+4.9, p_holm 3.3e-3). GPT-OSS-120B under a mixed prompt +7.2. HTML extract for these two cells. — [Pan26](https://arxiv.org/html/2609.09242v1)
- SHAP on repair: code tokens outrank comment tokens in nearly all instances (1,141 and 1,486 exact matches). "How" comments most important for both models. "What" least for CodeT5+; property and usage lower for DeepSeek. Without "how" comments EM is still 91% above baseline. Comments come from the fixed code, so "how" may simply leak the fix. — [Vitale](https://arxiv.org/abs/2601.23059)
- Types in completion, Code Match EM with comments, vs 12.81 / 16.01 / 11.39 removed: inline 14.07 / 18.15 / 14.44; Javadoc 12.33 / 16.3 / 12.78; multiline 13.4 / 15.46 / 13.4. Per-type comment-free columns repeat one value, so they do not look paired per subset (my reading). — [Imani](https://arxiv.org/abs/2512.16790)
- Probe accuracy for comment type as a concept: Javadoc median 96% with 76 pairs; inline and multiline 84% on the full set. This shows the model represents them, not that they help. — [Imani](https://arxiv.org/abs/2512.16790)
- Docstring contents, average pass@1 on HumanEval: original 34.9; examples and hints removed 33.0; one misspelling 33.0; distractor sentence 32.4; docstring gone 14.8. Oracle hints 42.6; oracle solution 70.4. SE about 3-4. Five of six models beat original on at least one worsened variant. `[pre-2024]` — [Dainese](https://aclanthology.org/2024.eacl-srw.24.pdf)
- Partial docstrings on real classes: full vs partial 0.15-2.72 points, 2 of 28 significant, negligible. RAG helped most with partial docstrings (+4.25 points mean; 5 of 7 models). — [RealClassEval](https://arxiv.org/abs/2510.26130)
- Partial docstrings on functions: GPT-4 full comments gave fewer errors than 10, 25 and 50% kept, "relatively minor". GPT-3.5 pattern unclear. Authors draw no conclusion. `[pre-2024]` — [Macke](https://arxiv.org/abs/2404.03114)
- Anecdotal agent failure classes: distraction 67%, anchoring 15% (a comment explaining a mechanism makes the agent keep it), editing complexity 12%, overgeneralization 6%. Denominator not stated, attribution method not stated. — [Antimemetics](https://antimemetics.institute/comment-ablation)

### Inferences
- Rule-relevant: a comment earns its place by stating something specific the code does not. Length and tone did not matter in the one test of them, so concise wording is safe and vague wording is not.
- A comment that restates the code has no direct measurement. The nearest evidence: code-derived content adds little (RealClassEval δ negligible, Vitale buggy-derived null), while content from outside the code adds a lot.
- Examples in docstrings were not separately valuable on HumanEval for 2023 models. No newer test exists.

### Gaps
- No experiment isolates why vs what vs contract vs example on real tasks with a strong model.
- Nothing on invariant or hazard comments, which is what a repository like this one mostly wants.
- Nothing on inline vs block placement beyond Imani's small, unpaired table.

## How many tokens does comment removal or code compression save, and does accuracy hold?

### Takeaway
**No study I found isolates comment removal and reports both token savings and task accuracy.** The closest evidence is formatting (24.5%, accuracy holds) and content-aware compression (up to 5.6×, accuracy mostly holds). Savings from comments alone depend on the repository's density, and it is large here.

### Cited Findings
- Removing indentation, whitespace and newlines, **comments explicitly excluded**: input tokens −24.5% average; Java 34.9, C++ 31.12, C# 25.26, Python 6.51. No Pass@1 drop above 4.2 points, and none significant. Claude-3.7 C++ 72.9 to 72.9 with −28.9% input; GPT-4o C++ 63.7 to 61.8 with −33.7%. N 314-355 per language, FIM task, 10 models. Output tokens only about −2.5 to −2.9%. Real files on Stack v2 (100k per language): 14.7, 13.2, 13.2, 4.0%. — [Hidden Cost](https://arxiv.org/abs/2508.13666)
- Asking the model to emit unformatted code is risky: Gemini-1.5 C++ 67.2 to 11.1 under the long prompt; GPT-4o C++ 61.8 to 65.9 with −32.6% output. — [Hidden Cost](https://arxiv.org/html/2508.13666v1)
- AST-identical grammar for Python (SimPy): 10.4% for GPT-4, 13.5% CodeLlama, up to 34.7%, on 623,887 files. Accuracy tested only on small models. Comments not addressed. ISSTA 2024. — [SimPy](https://arxiv.org/abs/2404.16333)
- Dropping 50% of code tokens by rule (SlimCode) on GPT-4: total tokens 114,404 to 83,267 (−27.22%), precision 0.85 to 0.87, n=400, Java. DietCode 50%: 0.56. Symbol tokens are 51.38% of tokens, removing them costs 2.83% MRR; identifiers are 15.48%, removing them costs 12.52% MRR (CodeBERT `[pre-2024]`). Comment handling not stated. FSE 2024. — [SlimCode](https://arxiv.org/abs/2405.11196)
- **Contradiction, same group.** SlimCode's paper: CodeBERT MRR −2.04% at 50% removal. LeanCode's later rerun: SlimCode 0.594, −18.18%, DietCode −40.90%, LeanCode −5.23%, base 0.726. SlimCode's paper had DietCode at −11.31%. Slightly different dataset sizes. Comment handling not stated. — [SlimCode](https://arxiv.org/abs/2405.11196); [LeanCode](https://arxiv.org/abs/2505.14759)
- Function and block selection (LongCodeZip): up to 5.6×; 77% input reduction on Qwen2.5-Coder. Claude-3.7-Sonnet ES 66.27 vs 66.24 at 4.3×; GPT-4o 64.72 vs 65.13. Seed-Coder EM 37.40 vs 40.20, a real drop. RepoQA DeepSeek-Coder 75.3 vs 38.3. Comment handling not specified. HTML for tables, headline in PDF. — [LongCodeZip](https://arxiv.org/html/2510.00446)
- Agent-level, task-aware line pruning: −23.1% tokens (Sonnet 4.5, 0.911M to 0.701M), −38.3% (GLM 4.6), success 70.6 to 72.0 (353 to 360/500) and 55.4 to 56.6. Cost −26.8% and −36.4%. Comments not treated separately. — [SWE-Pruner](https://arxiv.org/abs/2601.16746)
- Comments in model output: 45.5% of 88 models emit more than 20% comment tokens; 10.2% under 2%. Under "none", mean code tokens −21.8% (range −59% to +46%), not proportional to pass@1. — [Pan26](https://arxiv.org/abs/2609.09242)
- Tool READMEs claiming comment-stripping savings, for example over 50% token reduction, report keyword-overlap metrics rather than task success (search snippet only, page not opened). Not evidence. — [minicahe](https://pypi.org/project/minicahe/)
- **Local measurement, not literature.** Bytes inside comments in this repository, by a read-only script: `src/` 58.1% (550,381 of 947,487), `tests/` 32.9%, C/C++ under `tools/` 35.3%. Comment-only lines: 34.5% of `src/` lines (lines starting with `//`, `/*` or `*`, so slightly generous). Cross-check: `search.cpp` has 71.6% of bytes on full-line `//` comments. Bytes are a proxy, not tokens, and the script does not handle raw strings. — [script](/private/tmp/claude-501/-Users-max-ws-chesso-build/8e195c86-e693-45ac-b65e-6be5437b5a74/scratchpad/tools/comment_share.py)

### Inferences
- Formatting savings do not stand in for comment savings, and Hidden Cost's corpus figures exclude comment text from removal.
- For this repository, comments are a bigger lever than formatting: about 58% of `src/` bytes vs a C++ formatting saving near 31%. With reads at 67.5-76.1% of agent tokens (SWE-Pruner), removing all of them bounds savings at roughly 39-44% of total tokens. This is arithmetic, unmeasured, ignores extra exploration, and removing all is not what is being proposed.
- Accuracy held wherever the removed material was redundant to the task. It fell when removal took identifiers or specifics. Comments are not shown to be redundant in general, only on some tasks.
- A "don't write comments" instruction is not free: it can damage output for models that reason in comments. An instruction to keep them out of the final diff is a different thing and was not tested.

### Gaps
- Token share of comments in typical repositories, per a real tokenizer: no study found.
- Comment-only stripping vs pass rate on any benchmark: not found. The Antimemetics post gives no token, cost or step data.
- Prompt-cache interaction: not found.

## Are there results specific to agents that read windows of files through tools, or to context compaction?

### Takeaway
Nothing measures comments under windowed reads or compaction. Adjacent results show that reads dominate agent tokens, that dropping old observations is cheap and mostly safe, and that extra instructions raise cost.

### Cited Findings
- Read operations are 76.1% of tokens for Sonnet 4.5 (execute 12.1%, edit 11.8%) and 67.5% for GLM-4.6 on SWE-bench Verified. — [SWE-Pruner](https://arxiv.org/abs/2601.16746)
- Opinion from the same paper: static compressors "indiscriminately" remove comments regardless of task. Not tested for comments. — [SWE-Pruner](https://arxiv.org/abs/2601.16746)
- Observation masking vs no context management on SWE-bench Verified, N=500, SWE-agent, paired bootstrap: Qwen3-Coder-480B 53.4 to 54.8 (p=0.3856), cost $1.29 to $0.61 (−52.7%); Gemini 2.5 Flash 32.8 to 35.6, cost −56.1%; Gemini 2.5 Flash thinking 40.4 to 36.4 (p=0.0406), cost −57.1%. LLM summary on that last model 31.4 (−9.0 pp, p<0.0001). Comments not mentioned. — [Complexity Trap](https://arxiv.org/abs/2508.21433)
- Context files: LLM-written reduce resolution 0.5% and 2% on average (p 0.87, 0.37, not significant), steps +2.45 and +3.92, cost +20% and +23% (significant). Developer-written +2.4% (p 0.21), cost up to +19%. Sonnet-4.5 59.9 to 58.7 on SWE-bench; 73.2 to 65.2 LLM and 70.3 developer on the 138-issue set. Python, 12 repos plus SWE-bench. These are repo-level files, **not comments**. — [Evaluating AGENTS.md](https://arxiv.org/abs/2602.11988)
- Agent trace study, 557 sessions, 94,813 events, mostly Claude Code: "Docstrings, inline comments, and prose embedded in source files are invisible to our instrument." So it cannot answer the question. After a documentation read, the next event is a code edit with P=0.002. HTML. — [Gao & Chen](https://arxiv.org/html/2608.20195v1)
- Editing complexity, 12% of comment-caused failures: more comments made mechanical edits harder (escaping, heredocs). Denominator not stated. Mini-swe-agent edits through bash, so this may not transfer to edit tools. — [Antimemetics](https://antimemetics.institute/comment-ablation)

### Inferences
- A windowed reader pays for comments on each read of those lines, and masking then discards them. Nothing tests whether a comment outside the window changes outcomes, whether a file-head comment gets read at all, or whether comments survive compaction summaries.
- The AGENTS.md result is a caution by analogy only: instructions were followed, and following them cost more without clear gain. Comments that instruct the reader may behave the same way. Unmeasured.
- Masking's small accuracy cost suggests much of what was read was not needed later, which fits comments being read once. Indirect.

### Gaps
- Comment placement under partial reads: no study found.
- Comment retention through summarization or compaction: no study found.
- How many comment lines agents actually read per task: no instrumented measurement found.
- Whether agents edit or trust comments differently via grep hits than via full reads: no study found.

## Do results differ between small function-level benchmarks and real repository-level tasks?

### Takeaway
Yes. Function-level tests give large effects because the docstring is the specification. Repository-level tests give small, noisy or heterogeneous effects, and the direction Pan26 predicts holds but compresses. The repo-level evidence base is one blog post, one small class-generation replication, one null on class generation, and Imani's small completion deltas.

### Cited Findings
- Same removal, different scale: HumanEval −20.1 points average `[pre-2024]` — [Dainese](https://aclanthology.org/2024.eacl-srw.24.pdf); real classes 0.44-3.13 points, 0/28 significant on 2024-2025 models — [RealClassEval](https://arxiv.org/abs/2510.26130). Models differ too, so this is not a clean scale test.
- Benchmark gap: ClassEval pass 83.60-88.27% vs real classes 24.59-33.60%, Cliff's δ 0.46-0.64. — [RealClassEval](https://arxiv.org/abs/2510.26130)
- Pan26 at both scales: LCB correct +17.2 points, wrong-topic −20.8. RepoClassBench, Claude-Opus-4.7 source: Java correct 41 gained vs 16 lost (p=0.001); wrong-topic Java 18 vs 65 (p<0.001), C# 4 vs 16 (p=0.012); correct vs wrong-topic on same tasks 75 to 9 and 20 to 2 (p<0.001). Only 1 of 24 cells individually significant (Qwen3-8B Java wrong-topic −13.8, p_holm 0.022). Not agentic: writes one class from a description. — [Pan26](https://arxiv.org/abs/2609.09242)
- Agents, repo level: task effects across two models correlate at r ≈ 0.04 and agree in direction on 55% of tasks. `requests` improved most without comments; `matplotlib` suffered. No numbers. Authors abandoned an obfuscation arm because agents flagged the code as suspicious. — [Antimemetics](https://antimemetics.institute/comment-ablation)
- Harder set, bigger harm: MCC −21.3% on CRUXEval vs −29.4% on LiveCodeBench. — [CodeCrash](https://arxiv.org/abs/2504.14119)
- Scope warnings: Thornton is synthetic, 15-60 lines, and the author makes no claim about production code. ALIBI uses real C/C++ from MegaVul, but with an optimizing attacker. — [Thornton](https://arxiv.org/html/2602.16741v1); [ALIBI](https://arxiv.org/html/2607.24964)
- Identifier names, related: GPT-4o ClassEval class summarization 87.3 to 58.7 with names obfuscated; LiveCodeBench 73.4 to 70.1; execution prediction largely flat on DeepSeek V3 LCB (99.3 to 98.6 alpha-renamed). Authors say they "focus on identifiers since not all programs have comments", so comments are not controlled. — [When Names Disappear](https://arxiv.org/abs/2510.03178)
- Older related: PaLM 2 summarization BLEU-4 19.23 to 15.69 with adversarial function names; Llama 2 70B 22.41 to 21.23. Comments stripped as preprocessing, not a condition. `[pre-2024]` HTML. — [Haldar](https://arxiv.org/html/2404.08018)

### Inferences
- Do not cite HumanEval-style numbers to justify a repository rule. The docstring there is the task statement, which no real comment is.
- r ≈ 0.04 across models is the most decision-relevant number if it holds: which tasks comments help may not carry between models, so a rule tuned to one model's measured behaviour may not transfer. It comes from an unreviewed post with no N.
- Effect size shrinks partly from low baselines and limited headroom, as Pan26 reports for C# below 12%.
- For a repo whose agents can run tests, a false comment's cost may be lower than any study shows, since none lets the agent verify. Unmeasured. Conversely, a wrong comment about search behaviour in this project would not fail a unit test, only a later SPRT, which pushes the other way. Also unmeasured.

### Gaps
- No peer-reviewed repo-level comment ablation with an agent, with reported rates, N and test. A local A/B on this repository is the only way to get a directly applicable number.
- No longitudinal study: comments in a repo maintained mostly by agents, whether they rot faster, and what that does to later tasks. A related study of LLM-generated comments in repositories exists ([exploratory study](https://arxiv.org/html/2607.01867v1)); I did not read it.
- Nothing on C++ engines or heavily comment-dense repos, where this repository sits at about 58% of `src/` bytes.
