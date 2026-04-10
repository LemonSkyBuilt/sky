# Development Workflow

This repository uses `GitHub` for source control and `GitHub Actions` for CI.

## Main Rules

- Do not push directly to `main`.
- Start each change from an issue or another clearly scoped task.
- Create a short-lived branch such as `feat/...`, `fix/...`, or `chore/...`.
- Open a pull request back to `main`.
- Wait for GitHub Actions to pass before merging.
- Prefer `Squash and merge` to keep `main` history compact.

## Typical Flow

1. Sync local `main`.
2. Create a branch for one task.
3. Make the code or documentation changes.
4. Run local verification when applicable.
5. Push the branch.
6. Open a pull request.
7. Merge after review and passing checks.
8. Sync local `main` again.

## Local Commands

Start a new task:

```bash
git switch main
git pull --ff-only origin main
git switch -c feat/example-change
```

After making changes:

```bash
git add .
git commit -m "feat: example change"
git push -u origin feat/example-change
```

After the pull request is merged:

```bash
git switch main
git pull --ff-only origin main
git branch -d feat/example-change
```

## CI

The main CI workflow is defined in [`.github/workflows/ci.yml`](../.github/workflows/ci.yml).

Current baseline checks:

- Set up `JDK 21`
- Run `mvn verify`
- Upload built JAR artifacts
- Upload Surefire test reports

## Templates

The repository includes:

- Feature issue template
- Bug issue template
- Pull request template

Use them so each change has clear scope, validation notes, and risks.
