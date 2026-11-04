# HC-128 Known Answer Test Vectors

## Purpose

This directory stores reference vectors for validating the
group's HC-128 implementation. A vector is not considered
independently verified until its source and expected output
have been reviewed.

## Vector requirements

- Each key must contain exactly 16 bytes.
- Each IV must contain exactly 16 bytes.
- Hexadecimal values must use a documented byte order.
- Expected output must come from an independent, traceable source.
- Do not generate expected output using the implementation
  being tested.
- Record the source URL, source version or retrieval date,
  source SHA-256, reference case and verification status.
- Record who transcribed and who independently reviewed each vector.
- If a value cannot be verified, do not mark the vector as verified.

## Test case fields

The manifest records:

- id
- source
- downloaded_date
- source_sha256
- source_case
- key_hex
- iv_hex
- offset_bytes
- length_bytes
- input_hex
- expected_hex
- byte_order
- transcribed_by
- reviewed_by
- status

An empty input_hex means the test checks keystream output.

## Byte order and offsets

Record bytes explicitly in hexadecimal form.

The reference states that each printed 32-bit word is represented
as s3||s2||s1||s0, while keystream bytes appear in the order
s0, s1, s2, s3. Therefore, reverse the order of the four byte pairs
within each printed word. For example, 73150082 becomes 82001573.
Do not reverse the entire concatenated output.

For a non-zero byte offset, initialize HC-128 from the
beginning and consume the preceding bytes before comparing
the requested segment.

## Verification policy

The KAT runner must compare actual and expected output
byte by byte and report the first mismatch.

Pass one or more fixture files to the runner to execute the
complete local vector set in one invocation. The runner returns
non-zero if any case fails, while continuing to report the
remaining cases:

```text
build/test_kat.exe tests/vectors/hc128-kat-001-zero-key-zero-iv.txt tests/vectors/hc128-kat-002-zero-key-iv-first-byte-01.txt
```

A test case is not verified merely because the program
returns PASS. Its source and expected output must first
be checked independently. A successful structural check of
the manifest does not establish algorithm correctness.

## Status values

- SOURCE_TRANSCRIBED: transcribed from a cited source; independent review and runtime verification are pending.
- NOT_REVIEWED: no independent reviewer has verified the transcription.
- VERIFIED: source and expected bytes have been independently reviewed.
- PASS: the runtime KAT has executed and matched every expected byte.
- FAIL: the runtime KAT executed and found a mismatch.
- BLOCKED: required implementation or verification conditions are unavailable.
- NOT RUN: the test has not been executed.

Do not use PASS to describe a vector that has only been transcribed.
