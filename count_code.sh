#!/usr/bin/env zsh
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

if ! command -v cloc >/dev/null 2>&1; then
    echo "Error: cloc is not installed." >&2
    exit 1
fi

cloc --vcs=git \
    --not-match-f='^ui_.*\.h$'

echo "Code count completed."

cloc --vcs=git \
    --not-match-f='^ui_.*\.h$' \
    --exclude-dir=qcustomplot,qcustomplot_old

echo "Code count with exclusions completed."