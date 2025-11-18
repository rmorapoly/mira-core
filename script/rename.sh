#!/bin/bash
#set -e
cd ..

# ---- Arguments ----
REPO_NAME="$1"      # e.g. repo-name
REPO_NAME_SP="$2"   # e.g. "Repo Name"

if [[ -z "$REPO_NAME" || -z "$REPO_NAME_SP" ]]; then
  echo "Usage: $0 \"repo-name\" \"Repo Name\""
  exit 1
fi

# --- 1. Rename *.pro -> repo-name.pro ---
for f in *.pro; do
  [[ -f "$f" ]] || continue
  mv "$f" "$REPO_NAME.pro"
  echo "Renamed $f -> $REPO_NAME.pro"
done

# --- 2. Rename *_*.ts -> repo-name_*.ts ---
for f in *_*.ts; do
  [[ -f "$f" ]] || continue
  suffix="${f#*_}"            # keep everything after first _
  new="${REPO_NAME}_${suffix}"
  mv "$f" "$new"
  echo "Renamed $f -> $new"
done

# --- 3. Edit TS file contents ---
for f in ${REPO_NAME}_*.ts; do
  [[ -f "$f" ]] || continue
  # Replace language="something_en_US" to language="repo-name_en_US"
  sed -i "s/language=\".*_en_US\"/language=\"${REPO_NAME}_en_US\"/g" "$f"
  echo "Updated language in $f"
done

# --- 4. Edit only the FIRST PROJECT_NAME in version.pri ---
if [[ -f "version.pri" ]]; then
  REPL="PROJECT_NAME            = ${REPO_NAME_SP}"
  sed -i "0,/PROJECT_NAME[[:space:]]*=.*/s//$REPL/" version.pri
  echo "Updated first PROJECT_NAME in version.pri -> $REPL"
fi

echo "✅ Done!"
