#!/usr/bin/env python3
"""Static validation for HC-128 KAT fixtures and manifest.

This script checks the structural consistency of KAT fixtures against
the manifest, verifies source document SHA-256 integrity, and verifies
that malformed fixtures are rejected as expected by the test plan (T07-04).
"""

import csv
import hashlib
import os
import sys

REQUIRED_FIELDS = {
    "id",
    "key_hex",
    "iv_hex",
    "offset_bytes",
    "length_bytes",
    "expected_hex",
}


def is_valid_hex(s: str) -> bool:
    if len(s) % 2 != 0:
        return False
    try:
        bytes.fromhex(s)
        return True
    except ValueError:
        return False


def parse_fixture(path: str):
    """Parse fixture file and return (fields_dict, None) or (None, error_msg)."""
    fields = {}
    seen_keys = set()

    with open(path, "r", encoding="utf-8") as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if "=" not in line:
                return None, f"line {line_num}: missing '='"
            key, val = line.split("=", 1)
            key, val = key.strip(), val.strip()

            if key in seen_keys:
                return None, f"line {line_num}: duplicate field '{key}'"
            seen_keys.add(key)
            fields[key] = val

    missing = REQUIRED_FIELDS - set(fields.keys())
    if missing:
        return None, f"missing required field(s): {', '.join(sorted(missing))}"

    # Validate key_hex
    if not is_valid_hex(fields["key_hex"]) or len(fields["key_hex"]) != 32:
        return None, f"key_hex must be exactly 32 valid hex chars (16 bytes)"

    # Validate iv_hex
    if not is_valid_hex(fields["iv_hex"]) or len(fields["iv_hex"]) != 32:
        return None, f"iv_hex must be exactly 32 valid hex chars (16 bytes)"

    # Validate offset and length
    try:
        offset = int(fields["offset_bytes"])
        length = int(fields["length_bytes"])
        if offset < 0 or length <= 0:
            return None, "offset_bytes must be >= 0 and length_bytes > 0"
    except ValueError:
        return None, "offset_bytes and length_bytes must be non-negative integers"

    # Validate expected_hex
    if not is_valid_hex(fields["expected_hex"]):
        return None, "expected_hex contains invalid hex characters"
    if len(fields["expected_hex"]) != length * 2:
        return (
            None,
            f"expected_hex length ({len(fields['expected_hex']) // 2} B) does not match length_bytes ({length} B)",
        )

    return fields, None


def main() -> int:
    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_root = os.path.abspath(os.path.join(script_dir, ".."))

    vectors_dir = os.path.join(repo_root, "tests", "vectors")
    manifest_path = os.path.join(vectors_dir, "hc128-kat-manifest.csv")
    pdf_path = os.path.join(repo_root, "tools", "reference", "hc128-reference.pdf")

    if not os.path.exists(manifest_path):
        print(f"[FAIL] Manifest not found: {manifest_path}", file=sys.stderr)
        return 1

    fixture_map = {
        "HC128-001": os.path.join(vectors_dir, "hc128-kat-001-zero-key-zero-iv.txt"),
        "HC128-002": os.path.join(
            vectors_dir, "hc128-kat-002-zero-key-iv-first-byte-01.txt"
        ),
        "HC128-003": os.path.join(
            vectors_dir, "hc128-kat-003-key-first-byte-55-zero-iv.txt"
        ),
    }

    # 1. Check reference PDF hash if available
    if os.path.exists(pdf_path):
        with open(pdf_path, "rb") as pf:
            pdf_sha = hashlib.sha256(pf.read()).hexdigest()
    else:
        pdf_sha = None

    # 2. Check each vector against manifest
    with open(manifest_path, "r", encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            cid = row["id"]
            if cid not in fixture_map:
                print(f"[FAIL] Unknown case in manifest: {cid}", file=sys.stderr)
                return 1

            fix_file = fixture_map[cid]
            if not os.path.exists(fix_file):
                print(f"[FAIL] Fixture file missing: {fix_file}", file=sys.stderr)
                return 1

            fields, err = parse_fixture(fix_file)
            if err:
                print(f"[FAIL] Fixture {cid} error: {err}", file=sys.stderr)
                return 1

            # Check lengths
            k_len = len(fields["key_hex"]) // 2
            v_len = len(fields["iv_hex"]) // 2
            e_len = len(fields["expected_hex"]) // 2
            print(f"[OK] {cid}: key={k_len} B, IV={v_len} B, expected={e_len} B")

            # Check manifest matching
            if fields["id"] != row["id"]:
                print(f"[FAIL] ID mismatch for {cid}", file=sys.stderr)
                return 1
            if fields["key_hex"].lower() != row["key_hex"].lower():
                print(f"[FAIL] key_hex mismatch for {cid}", file=sys.stderr)
                return 1
            if fields["iv_hex"].lower() != row["iv_hex"].lower():
                print(f"[FAIL] iv_hex mismatch for {cid}", file=sys.stderr)
                return 1
            if fields["expected_hex"].lower() != row["expected_hex"].lower():
                print(f"[FAIL] expected_hex mismatch for {cid}", file=sys.stderr)
                return 1
            if int(fields["offset_bytes"]) != int(row["offset_bytes"]):
                print(f"[FAIL] offset_bytes mismatch for {cid}", file=sys.stderr)
                return 1
            if int(fields["length_bytes"]) != int(row["length_bytes"]):
                print(f"[FAIL] length_bytes mismatch for {cid}", file=sys.stderr)
                return 1

            if pdf_sha and row.get("source_sha256"):
                if pdf_sha.lower() != row["source_sha256"].lower():
                    print(
                        f"[FAIL] PDF SHA-256 mismatch for {cid}", file=sys.stderr
                    )
                    return 1

            print(f"[OK] {cid}: fixture matches manifest")

    # 3. Check negative test cases (T07-04)
    invalid_dir = os.path.join(vectors_dir, "invalid")
    if os.path.isdir(invalid_dir):
        for fname in sorted(os.listdir(invalid_dir)):
            if not fname.endswith(".txt"):
                continue
            fpath = os.path.join(invalid_dir, fname)
            fields, err = parse_fixture(fpath)
            if err is None:
                print(
                    f"[FAIL] Malformed fixture '{fname}' was unexpectedly accepted",
                    file=sys.stderr,
                )
                return 1
            print(f"[OK] Rejected {fname} as expected ({err})")

    print("[OK] Static validation passed; C runtime KAT NOT RUN")
    return 0


if __name__ == "__main__":
    sys.exit(main())
