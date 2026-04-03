# Writing Documentation

This guide covers how to document components, APIs, and features in this project.

## Component READMEs

Each component should have a `README.md` in its directory with these sections:

1. **Description**: What the component does in one or two sentences
2. **Public Functions**: List of public API functions with brief descriptions
3. **Configuration**: NVS keys or defines used by the component
4. **Example**: How to use it (reference code from main.c or tests)
5. **Errors**: Common error codes and what they mean

## Main README

Keep the main README focused on:
- Quick start for new developers
- System overview and architecture
- Build and flash instructions
- Troubleshooting for common issues
- Link to more detailed docs in `/docs` folder

## Markdown Basics

- Use code blocks for all code examples (C, bash, JSON)
- Inline code for file paths, function names, variable names
- Links as relative paths: `[docs guide](docs/guide.md)`
- Headers with `#`, `##`, `###` (not underlines)

## Keeping Docs Current

When changing code or APIs:
- Update the corresponding component README
- Update main README if architecture or build process changes
- Update this guide if documentation standards change

