#!/usr/bin/env bash
set -euo pipefail

demo=${1:?Usage: bash tests/test_demo.sh path/to/hc128_demo}
fixture=tests/vectors/hc128-kat-001-zero-key-zero-iv.txt
expected=$(awk -F= '$1 == "expected_hex" { sub(/\r$/, "", $2); print $2 }' "$fixture")
if [[ ! $expected =~ ^[0-9a-f]{128}$ ]]; then
    echo "FAIL: expected fixture must contain exactly 64 bytes of hex." >&2
    exit 1
fi

tmp_dir=$(mktemp -d)
trap 'rm -rf -- "$tmp_dir"' EXIT
"$demo" > "$tmp_dir/actual.txt" 2> "$tmp_dir/stderr.txt"
if [[ -s "$tmp_dir/stderr.txt" ]]; then
    echo "FAIL: successful demo wrote to stderr." >&2
    cat "$tmp_dir/stderr.txt" >&2
    exit 1
fi

# Windows text stdout may use CRLF; all other output bytes must match exactly.
sed 's/\r$//' "$tmp_dir/actual.txt" > "$tmp_dir/actual-lf.txt"
printf '%s\n' "$expected" > "$tmp_dir/expected.txt"
if ! cmp -s "$tmp_dir/expected.txt" "$tmp_dir/actual-lf.txt"; then
    echo "FAIL: demo output differs from HC128-001." >&2
    diff -u "$tmp_dir/expected.txt" "$tmp_dir/actual-lf.txt" >&2 || :
    exit 1
fi
echo "PASS demo stdout: 64 bytes match HC128-001, lowercase hex, one newline"

# Linux-only write failure check. This is not an algorithm KAT.
if [[ $(uname -s) == Linux && -c /dev/full ]]; then
    status=0
    "$demo" > /dev/full 2> "$tmp_dir/io-error.txt" || status=$?
    if [[ $status -ne 3 || ! -s "$tmp_dir/io-error.txt" ]]; then
        echo "FAIL: stdout write error must report an error and exit 3." >&2
        exit 1
    fi
    echo "PASS Linux stdout write failure: exit 3"
else
    echo "NOT RUN: Linux /dev/full write failure check (platform-specific)"
fi
