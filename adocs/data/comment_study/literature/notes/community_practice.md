# Comments in LLM-maintained repositories: community practice, and where the knowledge goes

Retrieved 2026-11-01. Each item carries a date and a tag. **[M]** measured (study, A/B, eval; limits stated). **[R]** reported first-hand experience, no controlled measure. **[O]** opinion or proposal. **[V]** the source sells or owns the thing discussed, treat as an interested party.

Source limits the writer must keep in view:

- **OpenAI's post was not read.** `openai.com/index/harness-engineering/` returned 403. Everything on it below is second-hand (InfoQ, Guo, a survey) or from Lopopolo's own talk and podcast, which were read as transcripts. Check the original before quoting it.
- **Reddit is absent.** Searches surfaced no r/ClaudeAI or r/ExperiencedDevs thread that could be read. The community debate below is Hacker News, GitHub issues and blogs, so it carries HN's skew.
- **Other pages not read:** ACM DOI for "When Comments Mislead" (403), PMC13503320 (captcha), `useautumn/autumn#4784` (404), `epitaph` on PyPI (page would not render). Items resting on a search snippet alone say so.
- **Quote mechanics.** HN and GitHub quotes came straight from their APIs. Web page quotes came through an extraction model capped near 125 characters, so long passages appear as fragments. Text marked "paraphrase" had no verbatim form available.
- **Books.** Ousterhout and Martin are quoted from their own public debate, primary. Book-only lines come through reader notes and are marked.

## 1. What do teams running agent-first codebases report?

### Takeaway
No first-hand account from the large agent-first teams that could be read (OpenAI's talks, Anthropic's project posts, Stripe) states a code-comment policy or a comment outcome. Comments are simply absent. What they report is a small index file, a versioned docs tree, progress files, git log, and enforced lints and tests. Numbers exist for scale and for one memory A/B, none for comment practice. On comments, the large organisations speak only through system prompts, and Anthropic reversed its own in July 2026. Smaller teams do state comment practice (§2), without numbers.

### Cited Findings

**OpenAI harness engineering (Lopopolo, 2026-02-11; original unread)**
- [R,V] 2026-02-21, InfoQ on the post: "roughly a million lines of code" in a "five-month internal experiment", no hand-written source. Paraphrase: docs in a structured directory of maps, execution plans and design specs; cross-linked docs enforced by linters and CI. Quotes no OpenAI text. — [InfoQ](https://www.infoq.com/news/2026/02/openai-harness-engineering-codex)
- [R,V] 2026-02-22, Guo reporting the post (paraphrase): small AGENTS.md linking to design docs, architecture maps, execution plans, quality grades, all versioned; a background agent scans for stale docs and opens cleanup PRs. — [Artificial Ignorance](https://www.ignorance.ai/p/the-emerging-harness-engineering)
- [R,V] 2026-03-12, survey attributing these phrases to the post: "what Codex cannot see does not exist"; Friday manual cleanup of "AI slop", replaced by "golden principles" and background scans; "doc-gardening" agents. Third-party attribution only. — [yage.ai survey](https://yage.ai/share/harness-engineering-survey-en-20260312.html)
- [R,V] Apr 2026 (page shows "Apr 7", year inferred), Lopopolo on Latent Space: "the models fundamentally crave text."; "all the things that we have encoded in docs and tests and review agents". Cost reported: first month and a half "10 times slower" than by hand. Rate reported: about 3.5 PRs per engineer per day, later 5–10. Follow-up work tracked "as notes in a markdown file." Co-host's description of the index: "a short one 100 line overall table of contents". Comments in that transcript mean PR review comments, not code comments. — [Latent Space](https://latent.space/p/harness-eng)
- [R,V] Jun 2026, Lopopolo talk, speech-to-text: agent's first step is to "ground itself in the documentation knowledge base"; "We want it to spider through our history of address and design docs" ("address" is likely ADRs); "I never want to give the same review feedback twice"; "having these failing checks tell the agent why they failed, and what to do instead"; "We don't want to jam a ton of rules in here". Extraction found no mention of comments or git history. — [transcript](https://tessl.io/registry/ainativedev/aidevcon-2026-ldn/files/talk-lopopolo-harness-engineering-humans-steer-agents-execute/transcript.md)

**Anthropic (interested party)**
- [R,V] 2026-07-24, Shihipar, the reversal. Old prompt: "In code: default to writing no comments. Never write multi-paragraph docstrings or multi-line comment blocks — one short line max." New prompt: "Write code that reads like the surrounding code: match its comment density, naming, and idiom." Why: old rule was "wrong" for "very complex code"; "newer models have better judgement and can handle these decisions well without explicit rules." — [Claude blog](https://claude.dev/blog/the-new-rules-of-context-engineering-for-claude-5-generation-models/)
- [M,V] Same post: "We removed over 80% of Claude Code's system prompt for models like Claude Opus 5 and Claude Fable 5 with no measurable loss on our coding evaluations." Suite, sample and variance undisclosed. Not a comment-specific result. Findings on over-constraint come from "reading transcripts of our own internal usage", no counts. — [Claude blog](https://claude.dev/blog/the-new-rules-of-context-engineering-for-claude-5-generation-models/)
- [R,V] 2025-11-26, Young: agents told to "Read the git logs and progress files to get up to speed on what was recently worked on" and to "commit its progress to git with descriptive commit messages"; feature list kept as JSON because "the model is less likely to inappropriately change or overwrite JSON files compared to Markdown files". Clean state means "orderly and well-documented". Qualitative only, no benchmarks. — [Anthropic](https://www.anthropic.com/engineering/effective-harnesses-for-long-running-agents)
- [R,V] 2026-02-05, Carlini, 100,000-line compiler, nearly 2,000 sessions, about $20,000: "maintain extensive READMEs and progress files"; "Claude will often maintain a running doc of failed approaches and remaining tasks"; each agent "will spend significant time orienting itself"; "LLM-written code frequently re-implements existing functionality". No discussion of comments. — [Anthropic](https://www.anthropic.com/engineering/building-c-compiler)
- [R,V] 2026-10-13, source control team: merges from "around 2,500 code changes merged per day" (May 2026) to "over 30,000 per day" (September); engineers "examine the intent of the change as opposed to reading every line of code". Heavy load "particularly about commit ancestry and blame"; "automated bisect and revert pipelines". Reasoning, transcripts, comments and docs are not discussed. — [Anthropic](https://www.anthropic.com/engineering/redesigning-source-control)

**Others**
- [M,V] 2026-01-15, GitHub Copilot memory, live A/B: merge rate 90% vs 83%; review positive feedback 77% vs 75%; both p<0.00001. Mechanism: memories hold "references to specific code locations that support each fact" and the agent "verifies the citations in real-time", correcting on contradiction. The strongest measured result here, and it concerns a store outside the source, not comments. — [GitHub](https://github.blog/ai-and-ml/github-copilot/building-an-agentic-memory-system-for-github-copilot/)
- [R] 2026-02-19, Stripe, "Over 1,300" merged minion PRs a week: "almost exclusively give minions context from files that are scoped to specific subdirectories or file patterns", because otherwise "the agent's whole context window would fill with rules before the agent even starts." — [Stripe](https://stripe.dev/blog/minions-stripes-one-shot-end-to-end-coding-agents-part-2)
- [R] 2026-02-05, Hashimoto: "Each line in that file is based on a bad agent behavior, and it almost completely resolved them all". Comments not mentioned. — [mitchellh.com](https://mitchellh.com/writing/my-ai-adoption-journey)
- [R] 2026-02-24, Vasilopoulos, 108,000-line C#, 283 sessions: one hot-memory constitution, 19 domain agents, 34 cold on-demand spec docs; four observational case studies, single author, no control. — [arXiv 2602.20478](https://arxiv.org/abs/2602.20478)
- [O] 2026-04-02, Böckeler sorts controls as computational, "deterministic and fast, run by the CPU", or inferential, "more non-deterministic"; linter messages with fixes are "a positive kind of prompt injection". Notes OpenAI and Stripe examples are second-hand summaries. — [martinfowler.com](https://martinfowler.com/articles/harness-engineering.html)

### Inferences
- Reported outcomes with a number: scale (OpenAI, Anthropic, Stripe), Copilot A/B, prompt removal. Everything linking a knowledge-placement practice to quality is assertion.
- Silence on comments in the agent-first accounts is itself data: the teams at largest scale steer through docs, lints and review agents, and do not present comments as a lever. It does not show they delete comments.
- Anthropic's reversal rests on model judgement, not on evidence that comments help. "Match its comment density" also means a repository's existing comment habits propagate, including history logs.

### Gaps
- OpenAI's exact wording, unread. From memory and unverified here: "one big AGENTS.md" failure, and a phrase that too much guidance becomes non-guidance. Do not cite without checking.
- No agent-first team found publishing comment density, comment churn, or a before/after on a comment policy.

## 2. What are the camps in the debate?

### Takeaway
**There is no consensus and the camps talk past each other.** The sharpest agreement across camps is narrow: narration and session history do not belong at the line. The live fight is over agent-written reasons. No camp has a controlled measurement of comments in an agent harness on real repositories. "Fewer" borrows from studies of wrong comments, "more" from anecdotes and narrow studies, "agent-specific" has none.

### Cited Findings

| Camp | Strongest claim | Evidence it actually cites |
|---|---|---|
| Fewer | comments rot and mislead; narration is noise | indirect studies on false or stale comments; complaints |
| More | comments are always-in-context, zero-infrastructure memory | anecdote; one narrow training study |
| Agent-specific | tagged anchors, grep-first | author says it helped; no numbers |
| Elsewhere | knowledge belongs in commits, ADRs, a docs store | proposals; one synthetic benchmark |

**Camp: fewer comments**
- [O,V] Claude Code prompt, ccVersion 2.1.161, community extraction: "Default to writing no comments. Only add one when the WHY is non-obvious: a hidden constraint, a subtle invariant, a workaround for a specific bug, behavior that would surprise a reader." — [archive](https://github.com/Piebald-AI/claude-code-system-prompts/blob/main/system-prompts/system-prompt-comment-why-only-guidance.md). Whether this still ships is unconfirmed; see the July 2026 reversal in §1.
- [O,V] Same archive: "Don't reference the current task, fix, or callers ("used by X", "added for the Y flow", "handles the case from issue #123"), since those belong in the PR description and rot as the codebase evolves." — [archive](https://github.com/Piebald-AI/claude-code-system-prompts/blob/main/system-prompts/system-prompt-comment-what-and-task-context-avoidance.md)
- [O,V] Codex prompts, undated repo files: "Usage of these comments should be rare." — [openai/codex](https://github.com/openai/codex/blob/main/codex-rs/core/gpt-5.2-codex_prompt.md). Gemini CLI, legacy prompt snippet: "Add code comments sparingly... *NEVER* talk to the user or describe your changes through comments." — [gemini-cli](https://github.com/google-gemini/gemini-cli/blob/main/packages/core/src/prompts/snippets.legacy.ts)
- [R] 2026-08-21, HN bryanlarsen: "Those massive comments generally refer to intermediate states that never shipped". — [HN](https://news.ycombinator.com/item?id=49393682). hamandcheese: "The story of how the code got to a certain state belongs primarily in git commit messages, not in comments." — [HN](https://news.ycombinator.com/item?id=49393920)
- [R] 2026-08-23, fooster: "hugely verbose trash that immediately becomes outdated and brittle and confusing for humans and Claude alike." — [HN](https://news.ycombinator.com/item?id=49406196). iammrpayments reports the same comments send Claude to irrelevant files. — [HN](https://news.ycombinator.com/item?id=49406067). fooster also calls this the thread's consensus, which the dissent below contradicts. — [HN](https://news.ycombinator.com/item?id=49411258)
- [R] 2026-08-21, amdsn: an adjustment gains "grandiose prose about "preventing the critical bug that shipped once."" — [HN](https://news.ycombinator.com/item?id=49393666). muglug: "if the code is actually incorrect, the comment can nevertheless bolster the case for it." — [HN](https://news.ycombinator.com/item?id=49393414)
- [O] 2026-03-16, Stetsenko: "Comments are mutable, routinely inaccurate, and cannot capture trajectory." No evaluation in the paper. — [arXiv 2603.15566](https://arxiv.org/html/2603.15566v1)
- [M,V] 2025-10, OX Security via Help Net Security: "Comments everywhere (90–100%)". Coverage gives 300+ repositories, 50 using AI tools. Security vendor; percentage seen only in the secondary article, method not read. — [Help Net Security](https://www.helpnetsecurity.com/2025/10/27/ai-code-security-risks-report/)

**Evidence the "fewer" camp leans on (all measured, none in an agent harness on a real repo)**
- [M] 2025-12 (FSE 2026), Huang et al., Copilot and Cursor: defective commented-out code in context raised defective output, "reaching up to 58.17 percent"; telling the tool to ignore it reduced defects by no more than 21.84%. — [arXiv 2512.20334](https://arxiv.org/abs/2512.20334)
- [M] 2025-04, Haroon et al., preprint: fault localization after mutation, misleading comments 24.55%, misleading variable names 28.7%, dead code 18.5%. Comments added, not removed; comparison only between mutation types here. — [arXiv 2504.04372](https://arxiv.org/html/2504.04372v1)
- [M] 2024-09, Radmanesh et al., human repos: inconsistent changes about 1.5× more likely to lead to a bug-introducing commit, strongest right after the inconsistency appears. — [arXiv 2409.10781](https://arxiv.org/abs/2409.10781)
- [M] Search snippet only, unread (ACM 403): outdated comments "tend to introduce a systematic misleading effect" in completion. — [DOI](https://doi.org/10.1145/3832783.3834359)
- [M] 2024-06, snippet only, older models: GPT-4 wrote wrong statements in "about one fifth of its generated comments" (540 Java methods). — [arXiv 2406.14836](https://arxiv.org/pdf/2406.14836)

**Camp: more comments**
- [R] 2026-09-22, HN CharlieDigital, the strongest statement: "Code comments are line-of-sight for agents and one of the cheapest, highest leverage ways to get better coding performance from AI because unlike skills that may or may not activate, comments end up in context". No numbers. — [HN](https://news.ycombinator.com/item?id=49800381)
- [R] 2026-03-11, same author, team prompt for C#: `<remarks>` holds "the "why"; provide domain or business context, chain of thought, and reasoning"; "The LLM is very good at keeping these up-to-date on refactors (we are still doing human code reviews)". — [HN](https://news.ycombinator.com/item?id=47334770)
- [O] 2026-04-20, Akita: "Let the agent comment. It knows what it's doing."; "don't prune the comments the agent writes"; "For the agent, the comment is the most accessible source during a tool call." Project metrics are self-reported and not tied to comments. — [akitaonrails.com](https://akitaonrails.com/en/2026/04/20/clean-code-for-ai-agents/)
- [R] 2026-08-22, skeledrew: "agent mentions it encountered X comments in the code and revised its action as a result (yes, I read live agent transcripts)." — [HN](https://news.ycombinator.com/item?id=49400409). 2026-08-21, o-o-: "its own, old comments have helped me/it solve new issues on more than one occasion." — [HN](https://news.ycombinator.com/item?id=49393641). jedberg: "The comments are for the next Claude". — [HN](https://news.ycombinator.com/item?id=49393679)
- [O] 2026-07-30, troupo: comments are useful to the model "given how many of them (especially Claude) only reads small chunks of files". — [HN](https://news.ycombinator.com/item?id=49114657)

**Evidence the "more" camp could lean on**
- [M] 2026-01 (ICPC 2026), Vitale et al., paraphrase of abstract: comments improve bug-fixing accuracy by up to threefold when present in both training and inference. Trained models on single methods with LLM-generated comments, not agents. — [arXiv 2601.23059](https://arxiv.org/abs/2601.23059)
- [M] 2025-04, Sabetto et al., pilot, one assembler module, four LLMs: quiz scores 96%/90% with comments vs 84% without; all comments false, 73% to 61%; 20% false, essentially unchanged. Authors: "preliminary findings that warrant more robust experimentation." — [arXiv 2506.11007](https://arxiv.org/html/2506.11007v1)
- [M] 2025-12 (ICSE), Imani et al.: toggling the internal "comment" concept shifts performance from -90% to +67% by model and task. Shows models weigh comments heavily, in both directions. — [arXiv 2512.16790](https://arxiv.org/abs/2512.16790)
- [M] Mixed prior work, snippet of Imani et al.'s related work, unread at source: incorrect comments "can substantially degrade" and missing ones had "relatively minor impact" (Macke and Doyle); another found "no statistically significant effect" (Nikiema et al.). — [arXiv 2512.16790](https://arxiv.org/pdf/2512.16790)

**Camp: comments written for agents**
- [R] 2025-06-07, diwank (Julep), origin of AIDEV: "Use `AIDEV-NOTE:`, `AIDEV-TODO:`, or `AIDEV-QUESTION:`"; "Keep them concise (≤ 120 chars)."; "Before scanning files, always first try to **locate existing anchors**"; "Do not remove AIDEV-NOTEs without explicit human instruction." Claim: "really made a difference", no number. — [blog](https://diwank.space/field-notes-from-shipping-real-code-with-claude), [HN](https://news.ycombinator.com/item?id=44213763)
- [R] Same thread, undated on the mirror read (2025): diwank: "the comments do start to pile up."; SatvikBeri: "the comments that tend to help Claude tend to be very different from what helps humans."; against: nxobject: "comments to 'AI' could just as be comments to 'READER'."; nilirl: "Lot of visual noise". — [HN mirror](https://hn.svelte.dev/item/44211417)
- [R] 2026-10-03, bushido versions principles that "need to be quoted in any comments" so comments move with them. Tool author. — [HN](https://news.ycombinator.com/item?id=49948420)
- [O] 2026-10-03, gregwebs, the counter-position: "I don't want there to be agent-only designated documentation." — [HN](https://news.ycombinator.com/item?id=49948336)

**Camp: put it elsewhere** (see §3 for detail)
- [O] 2026-03-05, Yagmin, founder: `/docs` is "a temporary solution"; a bad doc gives "no smoking gun"; proposes a code-mapped context layer. Thin evidence. Reply, Toutouxc: it describes "the concept of "code comments" after being translated through 35 languages." — [yagmin.com](https://yagmin.com/blog/your-docs-directory-is-doomed/), [HN](https://news.ycombinator.com/item?id=47334526)
- [O] 2026-09-21, Gross: "Markdown is now source code, _not_ documentation."; calls his convention "the weakest part of this essay". Reply, aDyslecticCrow: "prompt rot". — [htmx.org](https://htmx.org/essays/markdown-in-src/), [HN](https://news.ycombinator.com/item?id=49808383)
- [O,V] 2026-10-03, Liao: "agents don't need memory. They need documentation."; "Facts should be rewritten when truth changes. No appending." No measurements; promotes his tool. — [liao.gg](https://liao.gg/blog/agents-dont-need-memory)

**The control problem, across camps**
- [R] 2026-05-21, GitHub issue: "Claude adds explanatory "what" comments to generated code even when both the project CLAUDE.md and persistent memory explicitly instruct zero comments". — [#61305](https://github.com/anthropics/claude-code/issues/61305). 2026-06-07, same complaint, "leaking its chain of thoughts". — [#65961](https://github.com/anthropics/claude-code/issues/65961)
- [R] 2026-08-14, ACS_Solver: "Commenting is the one area in which I've been unable to get Claude to respect any rules."; worst class: "added parameter bar because we can no longer use the frob to automatically derive bar". — [HN](https://news.ycombinator.com/item?id=49297560)
- [R] **Conflict:** 2026-07-30, troupo: "latest codes don't write comments in code by default". — [HN](https://news.ycombinator.com/item?id=49114657). 2026-08-21, Ghinda: Codex "had fewer comments", experiments promised "soon", none shown. — [blog](https://allaboutcoding.ghinda.com/a-week-of-using-codex-more-than-claude/)

### Inferences
- History logs are the least defended class. Even the pro-comment camp argues for reasons, and the complaints target session narration. CharlieDigital's "chain of thought" is the nearest defence. So the reader's 22% sits on the uncontested side, while the 44% reasons and hazards sit in the middle of the fight.
- Reasons written by an agent need separating from reasons known to be true. Both camps report comments acting on the agent; the "fewer" reports add that the content may be inflated. The dispute is partly about which comments each side has seen.
- Studies in both directions show LLMs weigh comment text heavily. That supports neither "fewer" nor "more"; it raises the cost of a wrong comment.

### Gaps
- No controlled study found of comment density or comment class against agent outcome on a real repository.
- Who dissents within large teams is invisible; HN voices are mostly individuals. No Reddit thread could be read.
- `agent-annotations-spec` (Show HN 2026-05-01, 1 point) appeared and was not read; no sign of adoption. — [HN](https://news.ycombinator.com/item?id=47972957)

## 3. Where should each kind of knowledge live for an agent reader?

### Takeaway
Measured work covers context files, not the line-versus-doc choice. Explicit instructions get followed; overviews do not help; optional retrieval often does not happen. **Progressive disclosure has measured support for token cost and none for correctness, and one vendor eval goes against on-demand discovery.** Nothing measures agents following a pointer out of a code comment or choosing to read git history.

### Cited Findings

**Context files: the studies disagree on cost**
- [M] 2026-02 (v3 2026-09), Gloaguen et al., ETH: LLM-generated files -0.5% resolution on SWE-bench Lite (p=0.87), about -2% on CTXbench (p=0.37); developer files +2.4% (p=0.21). Cost up about 20–23% (generated), up to 19% (developer). Docs removed from the repo: generated files +2.7%, so they mainly help when little other documentation exists. Context files "are not effective at providing a repository overview." — [arXiv 2602.11988](https://arxiv.org/html/2602.11988)
- [M] Same paper, compliance: `uv` used about 1.6 times per instance when mentioned vs under 0.01 when not; repo tools 2.5 vs under 0.05. Agents follow stated instructions. — [arXiv 2602.11988](https://arxiv.org/html/2602.11988)
- [M] **Contradiction on cost:** 2026-01, Lulla et al., 10 repos, 124 PRs: median runtime 28.64% lower, output tokens 16.58% lower, completion similar. — [arXiv 2601.20404](https://arxiv.org/abs/2601.20404)
- [M] 2026-07, Khatri, 288 runs, 17 tasks, 3 repos, independent: "Context strategy does not measurably move correctness on either agent." Claude pass rates: none 53.3%, always-on 55.6%, selective 55.6%. Selective lowered Claude cache-creation tokens (p=0.001, 11/11 tasks). Does not measure whether agents read the retrieved files. Underpowered by its own admission on timing. — [arXiv 2607.27250](https://arxiv.org/html/2607.27250v1)

**Progressive disclosure and following pointers**
- [M,V] 2026-01-27, Vercel: "In 56% of eval cases, the skill was never invoked."; pass rates: baseline 53%, skill 53%, skill with explicit instruction 79%, `AGENTS.md` docs index 100%. Eval count undisclosed, one framework. Note the winner is an always-loaded index. — [Vercel](https://vercel.com/blog/agents-md-outperforms-skills-in-our-agent-evals)
- [M,V] 2025-11-24, Anthropic Tool Search: about 85% fewer tokens; tool selection "Opus 4 improved from 49% to 74%, and Opus 4.5 improved from 79.5% to 88.1%". Internal, undescribed; tools, not docs. — [Anthropic](https://www.anthropic.com/engineering/advanced-tool-use)
- [M] 2026-02 (v3 May), SWE-ContextBench, 1,100 base and 376 related tasks: accurate retrieved summaries help; unfiltered or wrong context gives limited benefit or worse. Figures not in the abstract. — [arXiv 2602.08316](https://arxiv.org/abs/2602.08316)
- [O,V] 2026-07-24, Shihipar: "Claude Code has gotten very competent at using progressive disclosure"; calling central repositories "A common myth"; "consider having a tree of files". No data. — [Claude blog](https://claude.dev/blog/the-new-rules-of-context-engineering-for-claude-5-generation-models/)
- [O,V] 2025-09-29, Anthropic: agents "maintain lightweight identifiers (file paths, stored queries, web links, etc.)"; no measured result in the post. — [Anthropic](https://www.anthropic.com/engineering/effective-context-engineering-for-ai-agents)
- [R] 2025-11-25, HumanLayer: "Prefer pointers to copies"; observed by logging proxy that Claude Code injects "IMPORTANT: this context may or may not be relevant to your tasks."; "< 300 lines is best" is community lore by its own account. — [HumanLayer](https://www.humanlayer.dev/blog/writing-a-good-claude-md)
- [R] 2026-10-06, kaydub, a one-day informal test: plain Claude with a short prompt outperformed a large review skill. — [HN](https://news.ycombinator.com/item?id=49985036)
- [O] 2026-02-05, Böckeler: no unit tests for context engineering exist; "we can never be certain of anything". — [martinfowler.com](https://martinfowler.com/articles/exploring-gen-ai/context-engineering-coding-agents.html)

**At the line, and in file headers**
- [R] 2026-09-22, CharlieDigital: "start of the file because it frequently uses `sed -n 1,200p` to read files" and "inside the body of the method because it may find by keyword and read a few lines past". — [HN](https://news.ycombinator.com/item?id=49800381)
- [R] 2026-09-22, aDyslecticCrow: "Block comments on top of source-files and functions are great, and already very useful to coding agents." — [HN](https://news.ycombinator.com/item?id=49808383)
- [R] 2026-04-01, danoandco: AGENTS.md carries the "may or may not be relevant" tag, so "ADR is rather something agents would not question"; docs are global, comments are local ("reason for a specific workaround"). Tool author. — [HN](https://news.ycombinator.com/item?id=47597110)
- [V] Docs, current at retrieval: "`.claude/rules/` with a `paths:` filter" so a rule "only loads when Claude is actually working there". Stripe reports the same at scale (§1). — [Claude Code docs](https://code.claude.com/docs/en/best-practices)

**Commit messages and git history**
- [R,V] 2025-11-26, Young: agents are *told* to read git logs (§1). — [Anthropic](https://www.anthropic.com/engineering/effective-harnesses-for-long-running-agents)
- [R] 2026-10-13, klustregrif: "The Agents quite actively use the history. It's more effective than any comments or separate documentation". No data. — [HN](https://news.ycombinator.com/item?id=50077991). 2026-09-26, senordevnyc: "an agent can chew through years of history and artifacts on a large codebase in a half hour". — [HN](https://news.ycombinator.com/item?id=49857547)
- [R] 2026-08-21, fpoling: "git log -p and similar are way more useful and reflect the real history rather than LLM inference about it." — [HN](https://news.ycombinator.com/item?id=49393687)
- [R] 2026-09-30, cerved, the most detailed account: "Claude tends to just narrate the change"; "Instruct an LLM to give reason and it'll give you reasons -- whether they are real or not."; forbidding lies failed; forcing it to source each rationale from the diff, session, log and blame, and to tag claims as inferred, "seems to have helped a lot". — [HN](https://news.ycombinator.com/item?id=49913216)
- [O] 2026-03-16, Lore proposes trailers `Constraint`, `Rejected`, `Directive`, `Not-tested`; ordinary messages are "nearly useless for understanding _why_". **No experiments**, only a proposed six-month design. — [arXiv 2603.15566](https://arxiv.org/html/2603.15566v1)
- [O] 2026-09-22, aDyslecticCrow: "Git history is a bit annoying to navigate". — [HN](https://news.ycombinator.com/item?id=49808768)

**ADRs and decision logs**
- [R] 2026-06-29, philbo: "1. Write ADRs (or get agents to write them) 2. Commit ADRs to git 3. Mention ADRs in AGENTS.md". — [HN](https://news.ycombinator.com/item?id=48715520). 2026-10-03, gregwebs: agents "seem to be able to find and use docs and keep them up to date". — [HN](https://news.ycombinator.com/item?id=49948336)
- [R] **Against:** 2026-10-04, pornel: "I've been bitten by agent-written ADRs. Agents carelessly add extrapolated details and speculated nice-to-haves that I never asked for, and this becomes a source of bloat that keeps coming back like a boomerang." — [HN](https://news.ycombinator.com/item?id=49949765)
- [R,V] 2026-10-26, sornaensis, tool author, path-scoped lookup `adrai relevant path/to/file.rs`: "absolutely improves cohesive development". — [HN](https://news.ycombinator.com/item?id=50241879)
- [O] 2026-03-11, postit: ADRs and specs "should live alongside each package". — [HN](https://news.ycombinator.com/item?id=47334460)

**Tests, lints, hooks**
- [V] Claude Code docs: "A line like `Always typecheck before you finish` in this file works most of the time, not every time."; a rule that can't bend "belongs in a hook". — [docs](https://code.claude.com/docs/en/best-practices)
- [R,V] Jun 2026, Lopopolo: checks that say why and what to do mean "it's able to self heal"; "We can kind of use tests on the AST". — [transcript](https://tessl.io/registry/ainativedev/aidevcon-2026-ldn/files/talk-lopopolo-harness-engineering-humans-steer-agents-execute/transcript.md)
- [O] 2026-04-02, Böckeler's caveat: relying on AI-generated tests "puts a lot of faith" in them. — [martinfowler.com](https://martinfowler.com/articles/harness-engineering.html). 2025-06-07, diwank, opposite: "If an AI tool touches a test file, the PR gets rejected." — [blog](https://diwank.space/field-notes-from-shipping-real-code-with-claude)

**Single-home discipline**
- [R] 2026-08-22, raicho's ruleset: "One-home rule: each topic lives in exactly one doc; everywhere else links to it, never copies it."; "*Code is ground truth.*". — [HN](https://news.ycombinator.com/item?id=49403468)

### Inferences

Synthesis, not a finding of any source:

| Kind of knowledge | Best-supported home | For | Contested by |
|---|---|---|---|
| What changed, per step | commit message, step file | kernel, Clean Code journal objection, Claude prompt, HN majority | Ousterhout via notes: comments, not commit log |
| Local reason or hazard | comment at the line | Google, Ousterhout, why-only rule; in context without a decision to look | rot; agent-invented reasons |
| Rule that cannot bend | lint, test, hook with a fix message | Claude docs, Lopopolo, Hashimoto | hard for subjective rules |
| Cross-cutting decision, rejected alternative | decision log, reached by an always-loaded index | Nygard, Lopopolo, Adelina benchmark (§4) | pornel; retrieval misses |
| Repo overview | short, or omit | Gloaguen | Lulla's efficiency gain |

- The mechanism behind "comment beats doc" is that a comment arrives without the agent deciding to fetch it. Vercel's 56% and CharlieDigital's "may or may not activate" point the same way. A pointer *inside* a comment needs that decision again, so it may behave like the optional skill and not like the comment. Untested.
- A context file and a code comment differ here: Gloaguen measures always-loaded text and finds a cost. A comment costs only when its region is read.
- Rationale in commits needs the same audit as rationale in comments; cerved's reasons problem is not specific to comments.

### Gaps
- No controlled ablation of git log or blame against agent success. Search found none.
- No trace study of how often an agent follows a `see DEC-n` comment or how often it runs `git log` or `git blame` unprompted.
- Unverified memory: an April 2025 Anthropic doc said Claude must be explicitly prompted to search git history for "why" questions. The current page no longer has it and search did not confirm it.

## 4. Tombstones, "do not change" markers, pointer comments, re-adding

### Takeaway
Reports split by form. Inline tombstones are widely disliked; a structured record of rejections has one strong but synthetic benchmark. Hazard comments are the class every side keeps, yet vendors and practitioners say a real prohibition needs a hook or lint. Pointer comments are directly disputed. **No measurement found of agents re-adding code after a comment was removed.**

### Cited Findings

**Tombstones and rejected ideas**
- [O,V] Claude Code prompt, ccVersion 2.1.53, extraction: "Avoid backwards-compatibility hacks like renaming unused _vars, re-exporting types, adding // removed comments for removed code". — [archive](https://github.com/Piebald-AI/claude-code-system-prompts/blob/main/system-prompts/system-prompt-doing-tasks-no-compatibility-hacks.md)
- [R] 2026-07-30, troupo: agents write "This was code that did X, it was now removed". — [HN](https://news.ycombinator.com/item?id=49114657). 2026-07-27, nickm12: it "falsely implies that you somehow need to know these details about the removed code". — [HN](https://news.ycombinator.com/item?id=49065957). 2026-08-21, eloisant: "just talk about the current state of the code!" — [HN](https://news.ycombinator.com/item?id=49393822)
- [R] **Direct clash, same thread.** 2026-08-22, bryanlarsen: "If you put a comment "don't do foo" in your code, you're biasing future readers, human or artificial, to think about foo". — [HN](https://news.ycombinator.com/item?id=49400199). skeledrew: "It's about biasing thought about foo in a particular direction. And I've seen it work multiple times". — [HN](https://news.ycombinator.com/item?id=49400409)
- [M] 2026-07-28, Adelina, six models, nine tasks per run, synthetic fixtures, LLM judge: baseline Opus 4.8 violated prior rejections on 3–4 of 9 tasks per run; "Zero re-proposals in 159 of 159 tasks" with rejections as a structured delivered field; Mem0 retrieval dropped the veto from 38% of contexts; a choices-only CONVENTIONS.md leaked on 4 of 6 models. Author: "the n is small". Hosted at robrain.dev, ownership not shown, so possibly interested. Tests delivered context, not comments. — [Substack](https://theailead.substack.com/p/why-does-your-ai-coding-agent-keep)
- [O,V] 2026-10-30, Imaino, sells RepoMemory: "AI coding agents repeat reverted changes because the reason for the revert isn't anywhere they look."; a rules file "stops being followed once it's long". Example is hypothetical. — [DEV](https://dev.to/dimaino/how-to-stop-ai-coding-agents-repeating-reverted-changes-3oc0)
- [R,V] 2026-10-04, stavros, tool author: records "the paths you didn't take"; "I've been using it for months and it works great." README sample: "Considered replacing SQLite FTS5 with Bleve for full-text search. Rejected."; stored in `.gnosis`, keyword search, and excludes what the code shows. — [HN](https://news.ycombinator.com/item?id=49957482), [gnosis](https://github.com/skorokithakis/gnosis)
- [R,V] Snippet only: `epitaph` keeps committed tombstones, "testimony, not a verdict", human-approved. — [PyPI](https://pypi.org/project/epitaph/0.1.0/). Lore's `Rejected:` trailer is the same idea in commits, unevaluated. — [arXiv 2603.15566](https://arxiv.org/html/2603.15566v1)

**Re-adding and re-deriving**
- [M] 2026-07-30, Ebrahimi et al., preprint, adjacent: models keep code a change requires removing. Deletion recall at most 71.7%; right file over 92% but exact line under 52%; 29.0% of passing patches wrap the target in a guard. Best model fails one task in five on deletion-only tasks. About code deletion, not comments. — [arXiv 2607.28887](https://arxiv.org/abs/2607.28887)
- [R,V] 2026-02-05, Carlini: "LLM-written code frequently re-implements existing functionality". — [Anthropic](https://www.anthropic.com/engineering/building-c-compiler). 2025-10-15, Böckeler, spec-kit: agent generated described classes "all over again, creating duplicates". — [martinfowler.com](https://martinfowler.com/articles/exploring-gen-ai/sdd-3-tools.html)
- [R] 2026-04-17, issue, moving code stripped comments including "casting here reduces TSServer diagnostic time significantly" and "@WARNING any nested entities may still be available". The model blamed its system prompt; that self-report is not evidence of cause. Closed. — [#49743](https://github.com/anthropics/claude-code/issues/49743)
- [R] 2026-04-20, Akita: "without the instruction it WILL prune them". — [akitaonrails.com](https://akitaonrails.com/en/2026/04/20/clean-code-for-ai-agents/)

**"Do not change" hazard markers**
- [V] Claude Code docs: "If Claude keeps ignoring a line that *is* in `CLAUDE.md`, adding IMPORTANT probably won't fix it."; emphasis "mostly makes Claude over-apply the rule." — [docs](https://code.claude.com/docs/en/best-practices)
- [O,V] 2026-07-24, Shihipar: CLAUDE.md should "spend most of the tokens on gotchas inside of the codebase". — [Claude blog](https://claude.dev/blog/the-new-rules-of-context-engineering-for-claude-5-generation-models/)
- [R] 2025-10-15, Böckeler: Tessl marks generated code `// GENERATED FROM SPEC - DO NOT EDIT`; separately she "frequently saw the agent ultimately not follow all the instructions", and saw it go "way overboard" on others. — [martinfowler.com](https://martinfowler.com/articles/exploring-gen-ai/sdd-3-tools.html)
- [M] Closest analogue: 2025-12, Comment Traps: an explicit "ignore this" reduced induced defects by at most 21.84%. Instructions are not a hazard marker, so this only suggests partial effect. — [arXiv 2512.20334](https://arxiv.org/abs/2512.20334)
- [O] Classic: Google tells reviewers to read prior comments for "a comment advising against this change being made". — [eng-practices](https://google.github.io/eng-practices/review/reviewer/looking-for.html)

**Pointer comments citing tickets or decisions**
- [O] **For:** Akita template: "Reference issue numbers / commit SHAs when a line exists because of a specific bug or upstream constraint." — [akitaonrails.com](https://akitaonrails.com/en/2026/04/20/clean-code-for-ai-agents/). CharlieDigital: "point is to a specific issue number or PR". — [HN](https://news.ycombinator.com/item?id=49800381)
- [O,V] **Against:** Claude Code prompt, §2: "handles the case from issue #123 ... rot as the codebase evolves". Comment-analyzer: "Avoid comments that reference temporary states or transitional implementations". — [plugin](https://github.com/anthropics/claude-plugins-official/blob/main/plugins/pr-review-toolkit/agents/comment-analyzer.md)
- [R] Pointer rot observed: rfx PR had to fix references to deleted scripts (§5). `/doctor prompt-audit` exists to flag instructions that "point at files that no longer exist". — [docs](https://code.claude.com/docs/en/best-practices)
- [M,V] Pointers plus verification: Copilot checks citations at read time and rewrites on contradiction; seeded false memories were caught. The only measured pointer scheme found. — [GitHub](https://github.blog/ai-and-ml/github-copilot/building-an-agentic-memory-system-for-github-copilot/)
- [O] Classic form, **unverified**: a search snippet of a Goodreads reader highlight has Ousterhout documenting cross-module issues "in a central file called designNotes". The Goodreads pages rendered empty when fetched, so neither the page nor the wording is pinned; check the book. A third-party skill describes the same pattern, labeled sections with short pointer comments in code (snippet). — [clairvoyance skill](https://www.claudepluginhub.com/skills/codybrom-clairvoyance/comments-docs)

### Inferences
- The prompt's objection is to references that die with the task. A stable, never-reused decision id resolves in the repository itself, closer to designNotes than to an issue link. That distinction is untested, but the "for" and "against" quotes do not address the same object.
- Rejections appear to work when delivered, and fail when retrieval has to find them. That argues for an index or a local comment over a store the agent must query. The benchmark is thin, so hold this loosely.
- A hazard stated in a comment informs; it does not bind. For the reader's 44% class, a hazard that has a test or a lint behind it survives an agent that ignores or strips the comment. #49743 shows the stripping happens.
- Removing history logs risks losing an embedded hazard. A log line may be the only record of a reason, so classify before deleting.

### Gaps
- No controlled evidence on whether an inline "do not do X" lowers or raises X. Clash above is anecdote against anecdote.
- No before/after report of agents re-deriving or re-adding after a comment purge.

## 5. Bulk comment cleanups by agents: how run, how verified, what went wrong

### Takeaway
Practice is young and visible mostly in PR bodies. AST identity is the established check, with a strong pre-LLM lineage. **No report found of a bulk comment cleanup breaking code; no first-hand report of token-stream comparison.** Reported failures are about the comments: under-deletion, regrowth, stripped reasons, and verification that says nothing about comment truth.

### Cited Findings

**How run, and how verified**
- [R] 2026-10-20, `bk-squared/rfx#1927`, Codex implementer, bot-merged: "every changed `.py` file (38) has an AST identical to the base" with docstrings stripped; falsifier "(0 differing files)"; "Review: skipped — (a) comment/docstring-only change". Scope: correcting stale text, including reproducing one docstring claim on main. Left three stale strings because they were executable. "35 of 36 checks passed" at merge. Claims not independently verified. — [PR](https://github.com/bk-squared/rfx/pull/1927)
- [R] 2026-10-30, `local-operator#3193`: table of finding, file:line, false statement, disposition; "Each item was checked against `origin/main` at 6091cbf6171 before it was edited." Verifies truth of comments, opposite emphasis to AST-only. — [PR](https://github.com/damianvtran/local-operator/pull/3193)
- [R] Snippet only (page 404 when fetched): `useautumn/autumn#4784`: each changed file parsed before and after with `@babel/parser`; "Because the ASTs are identical, the emitted code is unchanged." — [PR](https://github.com/useautumn/autumn/pull/4784)
- [R] 2026-07-10, boredtofears: "Look at all commits in the past X days and remove (DO NOT TRIM) all comments that are not truly exceptional"; "it's tough to lint something that subjective." — [HN](https://news.ycombinator.com/item?id=48861346)
- [V] Anthropic's `comment-analyzer`: "Cross-reference every claim in the comment against the actual code implementation."; flags "Comments that will become outdated with likely code changes"; "You analyze and provide feedback only. Do not modify code or comments directly." — [plugin](https://github.com/anthropics/claude-plugins-official/blob/main/plugins/pr-review-toolkit/agents/comment-analyzer.md)
- [V] Snippet only: community hook "yells at you when you write unnecessary comments" on PostToolUse. — [go-claude-code-comment-checker](https://github.com/code-yeongyu/go-claude-code-comment-checker)

**Pre-LLM precedents**
- [R] Black, formatter: "By default, Black performs an AST safety check after formatting your code." — [docs](https://black.readthedocs.io/en/stable/usage_and_configuration/the_basics.html). Its comparison special-cases Python docstrings because they are AST nodes: "Constant strings may be indented across newlines, if they are docstrings; fold spaces after newlines when comparing." — [source](https://github.com/psf/black/blob/main/src/black/parsing.py)
- [R] Linux `scripts/objdiff`: "a small script for validating that a commit or series of commits" "didn't change object code."; strips with `$STRIP -g $1 -R __bug_table -R .note -R .comment` and removes addresses from disassembly. — [kernel](https://raw.githubusercontent.com/torvalds/linux/master/scripts/objdiff)

**Tool blind spots (snippets only)**
- [R] Hashing a normalized AST ignores comment edits, so a comment-only stamp stays unchanged. — [PSIV#143](https://github.com/TusanHomichi/PSIV/issues/143). The reverse bug also exists: a structural fingerprint changed on comment text alone. — [W.H.A#4](https://github.com/Twix1288/W.H.A/issues/4). A validator that adds comments to its AST diff still passes a comment moved under another parent. — [sittir#727](https://github.com/refactory-lang/sittir/pull/727)

**What went wrong**
- [R] Under-deletion. 2026-08-14, droserasprout: "drop the comments from the current code changes" and "Claude instead just slightly trimmed them." — [HN](https://news.ycombinator.com/item?id=49298165)
- [R] Regrowth. ACS_Solver: it cleans up, "making a note in memory to comment less, and it still does the exact same thing next time." — [HN](https://news.ycombinator.com/item?id=49297560). #61305: "A reminder fixes it for that turn; the next code-generation turn regresses." — [issue](https://github.com/anthropics/claude-code/issues/61305)
- [R] Over-deletion of reasons during unrelated work: #49743 (§4).
- [R] Replacement text may be wrong. Cerved's reasons problem (§3); GPT-4's one-fifth figure (§2, snippet).

### Inferences
- AST or object identity proves the code did not move. It proves nothing about what was lost or whether the remaining comments are true. rfx skipped review on the strength of it, which covers the code risk and leaves the knowledge risk unreviewed. Three separate checks are needed: code unchanged, claims true, nothing unique deleted.
- For C++ (researcher's reasoning, unsourced): comments never reach the AST, so AST comparison is simple, but it and object comparison see only the active build configuration. Code in an inactive `#if` branch is unchecked, so each configuration must be compared, or lexer tokens compared with comments dropped and locations ignored. Deleting comment lines also shifts `__LINE__` and debug info, which would explain objdiff's stripping `-g` and `__bug_table`; an `assert` build can differ with no real change.
- Boredtofears' objection applies to *judging* value. A per-step history log has syntax, an id pattern, so unlike subjective comment quality it can be linted against regrowth. Combined with "match its comment density", a cleanup without a gate may regrow.

### Gaps
- No postmortem found of a large agent-run comment purge with follow-up outcomes.
- No first-hand report of token-identity verification; it is reasoned above, not reported.

## 6. The classic baseline, and which premises change

### Takeaway
The classics disagree with each other too: Martin treats comments as failure, Ousterhout as essential, Google asks for why, the kernel for what. They share a reader who has limited time, remembers things, and distrusts what is written. An LLM reader starts each session with no memory, pays per token, and acts on comment text even when it is wrong. Few of these replacements have been measured, and those only in narrow settings.

### Cited Findings

**What each says**
- [O] Ousterhout, debate held 2024-09 to 2025-02, primary: "Comments are crucially important and add enormous value to software."; "there is a lot of important information that simply cannot be expressed in code."; "It should be possible to use a method without reading its code."; on staleness: "Incorrect comments do happen, but I don't encounter them very often."; "For me the cost of missing comments is easily 10-100x the cost of incorrect comments." — [aposd-vs-clean-code](https://github.com/johnousterhout/aposd-vs-clean-code)
- [O] Ousterhout, book via reader notes: "Comments belong to the code, not the commit log." Verify against the book, ch. 16. — [Lebrero notes, 2021](https://danlebrero.com/2021/02/24/philosophy-of-software-design-summary/)
- [O] Martin, same debate: "Comments are always failures."; "I don't trust comments to be maintained, nor do I trust that they will be read."; "I look at every comment as potential misinformation."; yet "Nothing can be quite so helpful as a well placed comment." — [aposd-vs-clean-code](https://github.com/johnousterhout/aposd-vs-clean-code)
- [O] Clean Code (2008) via secondary: Journal comments are a log of every change added at the top of a module; on commented-out code, "good source code control systems ... will remember the code for us." These are the classic objections to the reader's history logs. Exact wording unverified. — [course slides](https://katie.cs.mtech.edu/classes/archive/f13/esof427/guru/comments.pdf), [Jablonski](https://world.hey.com/michael.jablonski/commented-out-code-8d21ef83)
- [O] Google C++ guide, current: "Comments are absolutely vital to keeping our code readable."; "write for your audience: the next contributor who will need to understand your code."; "Do not state the obvious."; "describe why the code does what it does". — [cppguide](https://google.github.io/styleguide/cppguide.html)
- [O] Google eng-practices: "Usually comments are useful when they explain why some code exists"; "mostly comments are for information that the code itself can't possibly contain," "like the reasoning behind a decision." — [eng-practices](https://google.github.io/eng-practices/review/reviewer/looking-for.html)
- [O] Linux coding style §8: "Comments are good, but there is also a danger of over-commenting."; "NEVER try to explain HOW your code works in a comment"; "Generally, you want your comments to tell WHAT your code does, not HOW."; "try to avoid putting comments inside a function body"; head of function: "what it does, and possibly WHY it does it." **Differs from Google's "why, not what".** — [kernel](https://www.kernel.org/doc/html/latest/process/coding-style.html)
- [O] Linux on commits: body goes in "the permanent source changelog" for "a competent reader who has long since forgotten the immediate details"; must be understandable "without external resources". — [submitting-patches](https://www.kernel.org/doc/html/latest/process/submitting-patches.html)
- [O] antirez, about Oct 2018 (page shows only relative age): nine types, function, design, why, teacher, checklist, guide, then questionable trivial, debt, backup; comments are "a tool for lowering the cognitive load of the reader." — [antirez](https://antirez.com/news/124)
- [O] Nygard, 2011-11-15: "One of the hardest things to track during the life of a project is the motivation behind certain decisions."; otherwise "Blindly accept the decision." or "Blindly change it."; "we will keep the old one around, but mark it as superseded." — [Cognitect](https://cognitect.com/blog/2011/11/15/documenting-architecture-decisions)

**Which premises change**

| Classic premise | Relied on by | With an LLM reader | Status |
|---|---|---|---|
| Reader retains context, can ask colleagues | all | "dropped into a fresh container with no context" ([Carlini](https://www.anthropic.com/engineering/building-c-compiler)); written record is the only transfer | Reported, solid |
| Reading cost is human time | Martin: "far more hours reading code"; Ousterhout: "50-80% of my development time" ([debate](https://github.com/johnousterhout/aposd-vs-clean-code)) | Cost is tokens: "In 2026 it's bad because it costs real money in tokens" ([Akita](https://akitaonrails.com/en/2026/04/20/clean-code-for-ai-agents/)). Reads are partial, which favours near-line placement ([troupo](https://news.ycombinator.com/item?id=49114657)) | Opinion; no study prices comment bytes |
| Comments go unmaintained | Martin | "with the agent able to rewrite the docstring alongside the code, that counter-argument lost weight" ([Akita](https://akitaonrails.com/en/2026/04/20/clean-code-for-ai-agents/)); **opposed:** "immediately becomes outdated" ([fooster](https://news.ycombinator.com/item?id=49406196)) | **Disputed, unmeasured** |
| Missing is worse than wrong | Ousterhout: "10-100x" | LLMs act on wrong text: misleading comments 24.55% ([Haroon](https://arxiv.org/html/2504.04372v1)); ignoring only partly works ([Comment Traps](https://arxiv.org/abs/2512.20334)); missing "relatively minor" per Macke and Doyle via snippet ([Imani](https://arxiv.org/pdf/2512.16790)); **opposed:** 84% vs 96% ([Sabetto](https://arxiv.org/html/2506.11007v1)) | Measured in narrow settings, may invert |
| A human reviewer filters comments | Google eng-practices | "Review: skipped" ([rfx](https://github.com/bk-squared/rfx/pull/1927)); "intent of the change as opposed to reading every line" ([Anthropic](https://www.anthropic.com/engineering/redesigning-source-control)); review agents instead ([Lopopolo](https://tessl.io/registry/ainativedev/aidevcon-2026-ldn/files/talk-lopopolo-harness-engineering-humans-steer-agents-execute/transcript.md)) | Reported; effect on comment quality unmeasured |
| Author knows why they wrote it | all "why" advice | Reason may be invented: "whether they are real or not" ([cerved](https://news.ycombinator.com/item?id=49913216)); inflated ([amdsn](https://news.ycombinator.com/item?id=49393666)) | Reported; new risk with no classic answer |
| History is there if you look | Clean Code on VCS; kernel on changelog | Fetching costs a decision and tool calls; agents are told to ([Young](https://www.anthropic.com/engineering/effective-harnesses-for-long-running-agents)) | Unmeasured |

### Inferences
- "Why, not what" survives the change of reader; every side quotes it, from the kernel's "possibly WHY" to the Claude prompt. Its justification changes: from saving a human's time to avoiding a wrong reconstruction.
- Ousterhout's "comments, not the commit log" and the prompt's "PR description" point opposite ways for the same text. They can be reconciled by kind: durable reason at the line, change story in the commit. That reconciliation is this writer's, not stated by either.
- The most consequential shift for the reader is the last-but-one row. Classic guidance assumes reasons are true because the author knows them. Where agents wrote the reasons and hazards (the reader's 44% of comment bytes), some unknown share may be confabulated, and neither AST checks nor age can reveal which.

### Gaps
- Ousterhout's book and Clean Code were not read directly; book-only lines are second-hand.
- No study comparing token cost of comments with exploration tokens they save. Liao's "about 80k tokens of exploration per session" is illustrative, not measured. — [liao.gg](https://liao.gg/blog/agents-dont-need-memory)
