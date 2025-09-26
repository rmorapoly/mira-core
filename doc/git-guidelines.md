# Git Guidelines

## Git Branching Model
  
At Mira, we follow a structured Git branching strategy (Git flow) to ensure clarity, stability, and parallel development. This model includes long-lived branches and short-lived prefixed branches for specific tasks.

<p align="center">
  <img src="img/git-branching.png" alt="git-branching.webp" style="zoom:50%;" />
</p>

---

## 📌 The Main Branches

The central repo holds two main branches with an infinite lifetime:

* `main` or `master`  — Stable, production-ready code. Every commit must be **tested** and **deployable**.
* `develop` — Latest delivered development changes for the next release. Integration branch for active development. New features and bugfixes are **merged** here. 

When `develop` is ready for release, it is merged into `master` and tagged.

---

## 🚧 Supporting Branches

Supporting branches may have limited lifetimes and may be removed eventually. They are:

*  **Prefixes**
   | Prefix        | Description |
   |---------------|-------------|
   | `feature/`    | For implementing new features. |
   | `bugfix/`     | For resolving non-critical issues in `develop`. |
   | `hotfix/`     | For urgent fixes to production (`main`) |
   | `release/`    | For new releases. |

1. **Feature branches**

   * Branch off: `develop`
   * Merge into: `develop`
   * Naming: `feature/*` The name contains the feature (e.g. `feature/ubx-decode`)
   * Purpose: Focusing on developing features rather than fixing bugs.
   * Trigger: New feature is going to develop.

2. **Bugfix branches**

   * Branch off: `develop`
   * Merge into: `develop`
   * Naming: `bugfix/*` The name contains the bug (e.g. `feature/serial-write`)
   * Purpose: Focusing on fixing bugs rather than developing features.
   * Trigger: A new bug is detected on `develop`, not on `feature/*` or `main`. Bugs in `feature/*` are part of the development process.

3. **Release branches**

   * Branch off: `develop`
   * Merge into: `develop`, `main`
   * Naming: `release/*` The name contains the version that will be released. (e.g. `release/1.0.2`)
   * Purpose: Preparing for the release by finishing the `README`, user manual, versioning, and verifying the wiki is accurate.
   * Trigger: New version is going to release.

4. **Hotfix branches**

   * Branch off: `main`
   * Merge into: `develop`, `main`
   * Naming: `hotfix/*` The name contains the version that will be released after the fixes (e.g. `release/1.0.2`)
   * Purpose: Fixing bugs missed during development and routine testing, often those discovered during long-term consistency checks or new scenarios; occasionally including urgent bug fixes.
   * Trigger: A new bug is detected on `main`.

---

## Release tags

   - Every release tag follows the format `major.minor.patch`, not just `major.minor` or `major` (e.g. `1.2.6`).
   - Each tag is created on the `main` branch and pushed to the `origin`.
   - Typically, the tag is made after a `release/*` or `hotfix/*` branch is merged into `main`.


---

## Feature Branch Workflow

**Create:**

```bash
$ git checkout -b feature/my-feature develop
```

**Merge:**

```bash
$ git checkout develop
$ git merge --no-ff feature/my-feature
$ git push origin develop
```

---

## Release Branch Workflow

**Create:**

```bash
$ git checkout -b release/1.2 develop
$ git commit -a -m "[DOC] Prepare version 1.2"
```

**Finish:**

```bash
$ git checkout main
$ git merge --no-ff release/1.2
$ git tag -a 1.2
$ git checkout develop
$ git merge --no-ff release/1.2
```

---

## Hotfix Branch Workflow

**Create:**

```bash
$ git checkout -b hotfix-1.2.1 main
$ git commit -a -m "[FIX] Fix old version and prepare version 1.2.1"
```

**Finish:**

```bash
$ git checkout main
$ git merge --no-ff hotfix/1.2.1
$ git tag -a 1.2.1
$ git checkout develop
$ git merge --no-ff hotfix/1.2.1
```

---

## Commit message conventions

You might feel **uncertainty** when encountering the Git commit message, unsure how to properly summarize the changes you've made and **why you've made them**. But the earlier in your career you can develop good committing habits, the better.

### Why should you write better commit messages?

The challenge is to open up a personal project or any repository for that matter and run `git log` to view a list of old commit messages. The vast majority of us who have run through tutorials or made quick fixes will say "Yep... I have absolutely no idea what I meant by 'Fix ui bug' 6 months ago." 
By writing good commits, you are simply future-proofing yourself. You could save yourself and/or coworkers hours of digging around while troubleshooting by providing that helpful description.

To come up with thoughtful commits, consider the following:

    Why have I made these changes?
    What effect have my changes made?
    Why was the change needed?
    What are the changes in reference to?

Assume the reader does not understand what the commit is addressing. They may not have access to the story addressing the detailed background of the change.

### Writing Clear Commit Messages

By consideration to the code to be self-explanatory. This is similar to the point above. 

See the differences below:

   ```bash
   git commit -m '[FET] Add margin'
   git commit -m '[FET] Add margin to nav items to prevent them from overlapping the logo'
   ```
Use **imperative mood (present tense)** in commit subject lines (Starting with capital letter).  
   
   ```bash
   git commit -m '[FIX] Resolve serial data parse crash in case of wrong data length received.'
   ```

For commit **description or body** you may use **past tense** to describe what happened in details.

   ```bash
   git commit -m '[BLD] Make Linux compatible' \
              -m 'Added DHCP support' \
              -m 'Adjusted paths for Linux build'
   ``` 

### Conventional Commits

Conventional Commit is a formatting convention that provides a set of rules to formulate a consistent commit message structure like so:

    [TYPE] Summary
    optional description

The commit type can include the following:

| Type      | Abbr| Description                                                                                      |
|-----------|-----|--------------------------------------------------------------------------------------------------|
| Feature   | FET | A new feature is introduced with the changes                                                     |
| Fix       | FIX | A bug fix has occurred                                                                           |
| Chore     | CHO | Changes that do not relate to a fix or feature and don't modify src or test files (e.g., updating dependencies or submodules) |
| Refactor  | REF | Refactored code that neither fixes a bug nor adds a feature                                      |
| Docs      | DOC | Updates to documentation such as the README or other markdown files or user manaual or wiki      |
| Style     | STY | Changes that do not affect the meaning of the code, likely related to formatting (whitespace, missing semicolons, etc.) |
| Test      | TST | Including new tests or correcting previous tests                                                 |
| Perfomance| PER | Performance improvements                                                                         |
| CI        | CIC | Continuous integration related changes                                                           |
| Build     | BLD | Changes that affect the build system or external dependencies (e.g., switching from msvc compiler to mingw or making the code cross-platform)                                   |
| Merge     | MER | A merge commit occured                                                                           |
| Revert    | REV | Reverts a previous commit                                                                        |

Here is an example:
   ```bash
   git commit -m '[FIX] Assign new thread for decoder because of race condition.'
   ```

Sometimes changes **do not fit neatly into a single type**. While you can **use multiple types** in one commit, it is **better to separate them into multiple commits** with focused purposes.

It is better to use belowing:

   ```bash
   git commit -m '[FET] Add new serial data parser ro manage data fragmentation.'
   git commit -m '[FIX] Eliminate the crash while using uninitialized address list.'
   ```

Insted of:

   ```bash
   git commit -m '[FET][FIX] Add new serial data parser ro manage data fragmentation and eliminate the crash while using uninitialized address list.'
   ```

### Writing Clear Commit Messages

- Each commit should represent **one logical change**.  
  - ✅ `[BLD] Make project compatible with Linux`  
  - ✅ `[BLD] Add DHCP support`  
  - ❌ `[BLD] Make Linux compatible, Add DHCP`

- If you need to include multiple closely related changes in one commit, prefer to:  
  1. **Separate them with `;`** in the subject:  
     - `Make Linux compatible; add DHCP`
  2. **Or use the commit body for details**:  
     ```
     Make Linux compatible

     - Added DHCP support
     - Adjusted paths for Linux build
     ```

- Remember: clear, atomic commits make history easier to understand and maintain.

Here are some overall commit messgae examples:

`[FET] Add local freq config`  
`[FIX] Change sensor data serial port to monitor temp`  
`[CHO] `

---

## Merging Base/Template Repo Updates

> **Important:** This repo may be integrated into other project as a separate remote.

- The first time you merge this repo into your project, Git may detect **unrelated histories**.
- To resolve this, use the command line:

```bash
git merge new-remote/main --allow-unrelated-histories


## Summary

This branching model is valuable because of its clear “big picture” structure. The visual diagram is an elegant mental model for branching and releasing processes and also writing good commit messages is an extremely beneficial skill to develop, and it helps you communicate and collaborate with your team.

---

## References

- [A Successful Git Branching Model by Vincent Driessen](https://nvie.com/posts/a-successful-git-branching-model/)
- [How to Write Better Git Commit Messages – A Step-By-Step Guide](https://www.freecodecamp.org/news/how-to-write-better-git-commit-messages/)
