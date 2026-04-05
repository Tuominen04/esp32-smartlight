---
description: "Use when writing documentation—READMEs, guides, component APIs. Keeps formatting and structure consistent."
name: "Documentation Guidelines"
applyTo: "docs/**, README.md"
---

# Documentation Checklist

## Markdown

Rules follow [markdownlint v0.40.0](https://github.com/DavidAnson/markdownlint/tree/v0.40.0/doc).

### Headings (MD001, MD003, MD018, MD022, MD023, MD024, MD025, MD026)

- Use ATX style only (`#`, `##`, `###`) — never setext underlines (MD003)
- One space after `#` — not `##Heading` (MD018) and not `##  Heading` (MD019)
- Heading levels increment by one at a time — never skip a level (MD001)
- Surround every heading with one blank line above and below (MD022)
- Headings must start at the beginning of the line — no indentation (MD023)
- No duplicate heading text at the same level (MD024)
- Only one top-level `#` heading per file (MD025)
- No trailing punctuation in headings — no `.`, `,`, `:`, `;`, `!` (MD026)

### Code Blocks (MD031, MD040, MD046, MD048)

- Always specify a language for fenced code blocks (MD040): ` ```c `, ` ```bash `, ` ```json `
- Use backtick fences (` ``` `) consistently — never tilde fences (MD048)
- Surround fenced code blocks with blank lines (MD031)
- Use fenced style for all code blocks — never indented style (MD046)

### Lists (MD004, MD007, MD029, MD030, MD032)

- Use `-` for all unordered list items — never `*` or `+` (MD004)
- Indent nested lists by 2 spaces (MD007)
- Surround lists with blank lines (MD032)
- One space after list markers (MD030)
- Ordered lists use `1.` prefix style (MD029)

### Links and Images (MD034, MD039, MD042, MD045, MD051, MD059)

- Links relative and lowercase: `[docs](docs/guide.md)`
- Never use bare URLs — wrap in angle brackets or use `[text](url)` (MD034)
- No spaces around link text — not `[ link ]` (MD039)
- No empty links — `[text]()` is invalid (MD042)
- All images must have alt text — `![description](image.png)` (MD045)
- Link fragments must match an existing heading anchor (MD051)
- Use descriptive link text — never `[click here]`, `[here]`, `[link]` (MD059)

### Whitespace and Structure (MD009, MD010, MD012, MD013, MD022, MD041, MD047)

- No trailing spaces on any line (MD009)
- No hard tabs anywhere — use spaces only (MD010)
- No more than one consecutive blank line (MD012)
- Maximum line length is 100 characters — URLs in links are exempt (MD013)
- First line of every file must be a top-level `#` heading (MD041)
- Every file must end with a single newline character (MD047)

### Emphasis (MD036, MD037, MD049, MD050)

- Do not use bold/italic as a substitute for headings (MD036)
- No spaces between emphasis markers and text — not `** bold **` (MD037)
- Use `*italic*` consistently — never `_italic_` (MD049)
- Use `**bold**` consistently — never `__bold__` (MD050)

### Inline Code

- Inline code for file names, function names, variables, NVS keys, and defines

## Component READMEs

Include:
- **Purpose**: One sentence
- **Public API**: Function signatures
- **Configuration**: NVS keys, defines
- **Errors**: ESP_ERR_* codes and what they mean
- **Example**: Code snippet showing usage

## Main README

- Features (plain text, no icons)
- Architecture diagram (ASCII)
- Quick start (prerequisites, hardware, build, flash)
- Project structure
- Common issues and troubleshooting
- Legal section

## Update Rule

When code changes, update corresponding docs immediately.
