---
description: "Check dependency updates, apply safe patch/minor upgrades, and report skipped major upgrades with risk notes."
name: "Update Packages"
argument-hint: "Target scope (whole workspace or path) and ecosystem (auto, npm, pip, poetry, dotnet, maven)"
agent: "agent"
---

# Update Packages

Update project dependencies with a safety-first workflow.

## Goal

- Apply patch/minor upgrades that are semver-safe.
- Keep major upgrades separate and clearly reported.
- Verify the project still works after updates.

## Step 1 - Detect package ecosystem and files

Inspect the workspace (or provided path) and detect dependency managers by lock/manifests:

- npm: `package.json`, `package-lock.json`, `npm-shrinkwrap.json`
- pip: `requirements.txt`, `requirements-*.txt`
- poetry: `pyproject.toml`, `poetry.lock`
- dotnet: `*.csproj`, `Directory.Packages.props`
- maven: `pom.xml`

If multiple ecosystems exist, process each and report separately.

## Step 2 - Check available updates

Run ecosystem-appropriate outdated commands and classify updates into:

- Patch/minor (safe by semver)
- Major (potential breaking)

Commands to use by ecosystem:

- npm:
  - `npm outdated`
- pip:
  - `python -m pip list --outdated`
- poetry:
  - `poetry show --outdated`
- dotnet:
  - `dotnet list package --outdated`
- maven:
  - `mvn -q versions:display-dependency-updates`

If a command is unavailable in the workspace, report that clearly and continue with what is available.

## Step 3 - Apply safe updates only

Apply only patch/minor updates unless the user explicitly asks for majors.

Recommended commands:

- npm: `npm update`
- pip (requirements-managed projects): update pinned versions in requirements files conservatively
- poetry: `poetry update` with constraints that avoid major jumps
- dotnet: update package references only when non-major
- maven: update dependency versions only when non-major

Never force major upgrades implicitly.

## Step 4 - Verify project health

Run the project's standard checks after updates.

- Use existing workspace test/build commands if present.
- Prefer lightweight verification first, then full suite when available.
- If checks fail, revert dependency manifest/lockfile changes and report the likely cause.

## Step 5 - Security check

Run ecosystem security checks where available and report production-impact findings first.

Examples:

- npm: `npm audit --omit=dev`
- pip: `pip-audit` (if installed)
- dotnet: `dotnet list package --vulnerable`

Do not use force flags that may introduce breaking changes.

## Output format

Provide a concise report with:

1. Summary
2. Updated packages table
3. Skipped major upgrades table
4. Verification results
5. Security findings

### Updated packages table

| Package | Old version | New version | Type | Ecosystem |
|---------|-------------|-------------|------|-----------|
| example | 1.2.0 | 1.5.0 | minor | npm |

### Skipped major upgrades table

| Package | Current | Latest major | Why skipped |
|---------|---------|--------------|-------------|
| example | 2.4.1 | 3.0.0 | Potential breaking changes |

## Rules

- Keep changes minimal and reversible.
- Prefer lockfile consistency.
- Do not add unrelated refactors.
- For personal/hobby projects, keep the report short and practical.
