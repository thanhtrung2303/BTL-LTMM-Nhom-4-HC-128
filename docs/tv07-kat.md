# Bàn giao TV07 Nguyễn Anh Tú — Known Answer Test (KAT)

TV07 phụ trách bộ test vector chuẩn (KAT), chỉ mục nguồn gốc dữ liệu
(`hc128-kat-manifest.csv`), bộ kiểm thử âm tính cho parser, chương trình chạy
KAT bằng C (`tests/test_kat.c`) và kế hoạch kiểm thử (`docs/test-plan.md`).

Sau khi ghép các module `hc128_init` (TV03), `hc128_core` (TV04),
`hc128_stream` (TV05) và `hc128_helpers` (TV02), cả ba vector trong Appendix A
của đặc tả tác giả đều chạy đạt qua binary C `build/test_kat` và khớp từng byte.

## 1. Các file phụ trách

| File | Mục đích |
|---|---|
| `tests/vectors/hc128-kat-001-zero-key-zero-iv.txt` | Vector 1: Key toàn 0, IV toàn 0 (64 byte) |
| `tests/vectors/hc128-kat-002-zero-key-iv-first-byte-01.txt` | Vector 2: Key toàn 0, IV0 = 1 (64 byte) |
| `tests/vectors/hc128-kat-003-key-first-byte-55-zero-iv.txt` | Vector 3: Key K0 = 0x55, IV toàn 0 (64 byte) |
| `tests/vectors/hc128-kat-manifest.csv` | Chỉ mục nguồn, SHA-256 PDF, quy ước byte và trạng thái |
| `tests/vectors/invalid/*.txt` | 6 fixture lỗi để kiểm tra parser từ chối input sai (T07-04) |
| `tests/vectors/README.md` | Quy định định dạng vector, thứ tự byte và cách chạy |
| `tests/test_kat.c` | Runner C đọc fixture, kiểm tra hợp lệ và so sánh từng byte |
| `tests/validate_kat.py` | Script kiểm tra tĩnh manifest/fixture và chạy kiểm thử T07-01..T07-04 |
| `docs/test-plan.md` | Ma trận kiểm thử T01–T10 của nhóm |
| `results/kat_log.txt` | Log chạy thực tế `make test-kat` |

## 2. Nguồn dữ liệu và quy ước chuyển đổi byte

- **Tài liệu gốc:** Hongjun Wu, *The Stream Cipher HC-128*, eSTREAM Phase 3,
  Appendix A (trang 9–10).
- **URL:** `https://www3.ntu.edu.sg/home/wuhj/research/hc/hc128.pdf`
- **Bản lưu trong repo:** `tools/reference/hc128-reference.pdf`
- **SHA-256:** `cfa5d5ff456691468f3e31d68e0c6a76181641bda352b22a107372e765d904a0`

### Quy tắc thứ tự byte (Little-Endian theo word 32-bit)

Appendix A biểu diễn mỗi từ khóa dòng 32-bit dưới dạng `S = s3 || s2 || s1 || s0`,
trong đó `s0` là byte có trọng số thấp nhất (LSB) và xuất hiện đầu tiên trong dòng
byte keystream (`s0, s1, s2, s3`). Do đó, khi chuyển từ các word 32-bit in trong
đặc tả sang chuỗi byte hex của fixture, ta đảo thứ tự 4 cặp hex trong từng word
32-bit (không đảo toàn bộ chuỗi 64 byte):

- Word đầu của Vector 1 trong tài liệu: `73150082` -> byte keystream: `82001573`.
- Word đầu của Vector 2 trong tài liệu: `c01893d5` -> byte keystream: `d59318c0`.
- Word đầu của Vector 3 trong tài liệu: `518251a4` -> byte keystream: `a4518251`.

Tương tự đối với Key và IV: trong Vector 2 (`IV0 = 1`), từ 32-bit `0x00000001`
đặt ở vị trí `IV0` tương ứng với chuỗi byte little-endian `01000000...`; trong
Vector 3 (`K0 = 0x55`), từ 32-bit `0x00000055` tương ứng với `55000000...`.

## 3. Cách chạy kiểm thử KAT

Tại thư mục gốc của repository:

```bash
make test-kat
python3 tests/validate_kat.py
```

- `make test-kat` biên dịch `build/test_kat` và chạy đối chiếu cả 3 fixture chuẩn,
  ghi kết quả vào `results/kat_log.txt`.
- `python3 tests/validate_kat.py` kiểm tra tính nhất quán giữa các file fixture và
  `hc128-kat-manifest.csv`, xác minh SHA-256 của PDF gốc, đồng thời gọi
  `build/test_kat` để kiểm tra cả 3 vector hợp lệ lẫn việc từ chối 6 fixture lỗi
  trong `tests/vectors/invalid/`.

## 4. Kết quả nghiệm thu T07 (Input / Expected / Actual)

| Mã test | Mô tả | Kết quả |
|---|---|---|
| T07-01 | KAT với zero key / zero IV (`HC128-001`, 64 byte) | **PASS** (khớp 64/64 byte) |
| T07-02 | Tối thiểu 3 cặp key–IV độc lập (`HC128-001`, `HC128-002`, `HC128-003`) | **PASS** (khớp cả 3 vector, mỗi vector 64 byte) |
| T07-03 | Vector có offset > 0 | **N/A** (đặc tả gốc chỉ công bố từ offset 0; code runner đã hỗ trợ sẵn `offset_bytes`) |
| T07-04 | Từ chối fixture thiếu trường, sai độ dài key/IV, hex lỗi, lệch `length_bytes`, lặp trường | **PASS** (parser C từ chối toàn bộ 6/6 file lỗi với mã thoát != 0) |

### Bảng đối chiếu 3 vector (64 byte đầu)

1. **HC128-001** (`Appendix A, vector 1`)
   - **Key:** `00000000000000000000000000000000`
   - **IV:** `00000000000000000000000000000000`
   - **Expected:** `82001573a003fd3b7fd72ffb0eaf63aac62f12deb629dca72785a66268ec758b1edb36900560898178e0ad009abf1f491330dc1c246e3d6cb264f6900271d59c`
   - **Actual:** `82001573a003fd3b7fd72ffb0eaf63aac62f12deb629dca72785a66268ec758b1edb36900560898178e0ad009abf1f491330dc1c246e3d6cb264f6900271d59c`
   - **Kết quả:** `[PASS] HC128-001: 64 bytes matched`

2. **HC128-002** (`Appendix A, vector 2`)
   - **Key:** `00000000000000000000000000000000`
   - **IV:** `01000000000000000000000000000000`
   - **Expected:** `d59318c058e9dbb798ec658f046617642467fc36ec6e2cc8a7381c1b952ab4c923f13e328b906a0a687b75cebbf7149f11e0cde43f17b5ae948c6089ca46cfb5`
   - **Actual:** `d59318c058e9dbb798ec658f046617642467fc36ec6e2cc8a7381c1b952ab4c923f13e328b906a0a687b75cebbf7149f11e0cde43f17b5ae948c6089ca46cfb5`
   - **Kết quả:** `[PASS] HC128-002: 64 bytes matched`

3. **HC128-003** (`Appendix A, vector 3`)
   - **Key:** `55000000000000000000000000000000`
   - **IV:** `00000000000000000000000000000000`
   - **Expected:** `a45182510a93b40431f92ab032f039067aa4b4bc0b482257729ff92b66e5c0cd560c0f31e883ccd3efb83d667fe0df6290173e599caacec56f8003aba0e5a6c9`
   - **Actual:** `a45182510a93b40431f92ab032f039067aa4b4bc0b482257729ff92b66e5c0cd560c0f31e883ccd3efb83d667fe0df6290173e599caacec56f8003aba0e5a6c9`
   - **Kết quả:** `[PASS] HC128-003: 64 bytes matched`

## 5. Review chéo phần TV06 (Demo / CLI)

Theo phân công trong `docs/team-plan.md`, TV07 là reviewer của TV06:

- Đã chạy `make -f app/Makefile.demo test` và `./build/hc128`: chương trình xuất
  đúng 128 ký tự hex chữ thường (64 byte) của `HC128-001`, kết thúc bằng một
  newline và mã thoát `0`.
- `app/main.c` chỉ gọi public API (`hc128_init`, `hc128_keystream`), không can
  thiệp trực tiếp vào bảng nội bộ `P`/`Q`.
- Lưu ý phạm vi hiện tại: `app/main.c` mới hoàn thành bản demo cố định key/IV
  (mốc ưu tiên), chưa có parser tham số dòng lệnh `--key`, `--iv`, `--bytes`
  cho các case T06-01..T06-04.

## 6. Nội dung tóm tắt cho Slide 13–14 và lời trình bày (khoảng 1 phút)

- **Slide 13 (Nguồn KAT & Quy ước byte):**
  - Nguồn chuẩn: Đặc tả eSTREAM HC-128 của Hongjun Wu (Appendix A), kiểm định
    toàn vẹn file bằng SHA-256.
  - Quy ước Little-Endian: Mỗi word 32-bit `s3||s2||s1||s0` xuất ra dòng khóa
    theo thứ tự `s0, s1, s2, s3` (ví dụ `0x73150082` -> `82 00 15 73`).
  - Cấu trúc kiểm thử: Tách biệt dữ liệu (`tests/vectors/`), chỉ mục quản lý
    nguồn (`hc128-kat-manifest.csv`) và chương trình kiểm tra (`tests/test_kat.c`).
- **Slide 14 (Kết quả đối chiếu Input / Expected / Actual):**
  - Ba bộ vector chuẩn (64 byte/vector):
    - `HC128-001`: Key = 0, IV = 0 -> Khớp 64/64 byte (`82001573...7102` -> `...0271d59c`).
    - `HC128-002`: Key = 0, IV0 = 1 -> Khớp 64/64 byte (`d59318c0...46cfb5`).
    - `HC128-003`: Key0 = 0x55, IV = 0 -> Khớp 64/64 byte (`a4518251...e5a6c9`).
  - Kiểm thử âm tính (T07-04): Parser C từ chối toàn bộ 6/6 fixture sai định dạng.

- **Đoạn trình bày khoảng 1 phút:**
  “Em phụ trách phần Known Answer Test (KAT) và kế hoạch kiểm thử của nhóm. Toàn bộ
  vector chuẩn được trích xuất độc lập từ Appendix A trong đặc tả HC-128 của tác
  giả Hongjun Wu và kiểm tra mã băm SHA-256 của tài liệu gốc. Do đặc tả in mỗi
  word 32-bit theo thứ tự byte cao đến thấp nhưng dòng khóa xuất byte thấp trước,
  từng word 32-bit được đảo thứ tự 4 cặp byte sang dạng little-endian. Em đã xây
  dựng 3 bộ fixture chuẩn 64 byte cùng chương trình C `test_kat.c` và 6 bộ fixture
  lỗi để kiểm tra khả năng bắt lỗi của parser. Sau khi tích hợp với các module
  khởi tạo, lõi và sinh dòng khóa của nhóm, cả 3 vector đều khớp tuyệt đối từng
  byte và toàn bộ các trường hợp dữ liệu lỗi đều bị từ chối đúng theo thiết kế.”
