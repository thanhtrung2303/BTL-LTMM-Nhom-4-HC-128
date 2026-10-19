 # HC-128 Known Answer Test Vectors

## Purpose

This directory stores independently verified test vectors
used to validate the group's HC-128 implementation.

## Vector requirements

- Each key must contain exactly 16 bytes.
- Each IV must contain exactly 16 bytes.
- Hexadecimal values must use a documented byte order.
- Expected output must come from an independent, traceable source.
- Do not generate expected output using the implementation
  being tested.
- Record the source, version, reference case and verification
  status for every vector.
- If a value cannot be verified, mark the case as BLOCKED.

## Test case fields

Each case should record:

- id
- source
- source_case
- key_hex
- iv_hex
- offset_bytes
- length_bytes
- input_hex
- expected_hex

## Byte order and offsets

Record bytes explicitly in hexadecimal form.

If a reference publishes 32-bit words, convert each word
to bytes according to the documented reference convention.
Do not reverse an entire output string without justification.

For a non-zero byte offset, initialize HC-128 from the
beginning and consume the preceding bytes before comparing
the requested segment.

## Verification policy

The KAT runner must compare actual and expected output
byte by byte and report the first mismatch.

A test case is not verified merely because the program
returns PASS. Its expected value and source must first
be checked independently.

## Status

New vectors remain unverified until their source and expected
output have been checked and reviewed.
