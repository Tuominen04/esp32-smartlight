---
description: "Use when writing documentation—READMEs, guides, component APIs. Keeps formatting and structure consistent."
name: "Documentation Guidelines"
applyTo: "docs/**, components/**/README*.md, README.md"
---

# Documentation Checklist

## Markdown

- Use `#`, `##`, `###` headers (no underlines)
- Code blocks with language: ` ```c `, ` ```bash `, ` ```json `
- Inline code for file names, functions, variables
- Links relative and lowercase: `[docs](docs/guide.md)`

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
