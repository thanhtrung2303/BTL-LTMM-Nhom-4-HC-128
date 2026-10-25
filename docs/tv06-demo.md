# Demo HC-128 của TV06 Đặng Quốc Anh

Bản demo thực hiện phần ưu tiên của TV06 trong bảng phân công: dùng key và IV
cố định, khởi tạo một lần, sinh 64 byte keystream và in hex. Chương trình đã
được kiểm tra trên Linux bằng vector của tác giả. CLI đầy đủ và parser thuộc
giai đoạn tiếp theo.

## File và phần đóng góp

| File | Nội dung |
|---|---|
| `app/main.c` | Demo TV06, chỉ gọi public API |
| `app/Makefile.demo` | Build và chạy demo/test riêng trên Windows UCRT64 hoặc Linux |
| `tests/test_demo.sh` | So stdout với fixture TV07, kiểm tra định dạng và lỗi ghi trên Linux |
| `tests/test_demo_api.c` | Ba KAT qua public API, chunk/reset, NULL và load little-endian |
| `results/tv06-demo-test.txt` | Lệnh, môi trường và kết quả thực tế |

Để demo có thể liên kết với các module thật, bản bàn giao ghép thêm hai phần
phụ thuộc chưa có đủ trên `main` nền `9dbc8b6d273450eda22860d39c5a98474b8aff56`:

- `src/hc128_init.c` là **nguyên bản của TV03**, lấy từ commit
  `39ec07db6a5c7f7ab95b607144c2714484f45c86` trên nhánh `feat/tv03-hc128-init`.
  Blob vẫn là `e4966a3a97fbeb6049571360ea9002d560be5dd9`; đây không phải mã mới
  do TV06 viết. TV03 tiếp tục phụ trách phần khởi tạo.
- `src/hc128_internal.h` bổ sung đúng ba helper mà init đang gọi:
  `hc128_load32_le`, `hc128_f1`, `hc128_f2`. Công thức được đối chiếu với
  mục 2.1 và 2.2 của [đặc tả tác giả](../tools/reference/hc128-reference.pdf).
  Đây là phần hỗ trợ tích hợp, cần TV02/TV03 review trước khi merge.

Public header, core TV04, stream TV05 và Makefile chung giữ nguyên. Makefile
riêng giúp TV08 xem xét cách ghép demo trước khi tích hợp vào build chung.

## Chạy trên Windows

Mở terminal **MSYS2 UCRT64** tại thư mục gốc repository, trên nhánh chứa bản
demo này. Chạy từng lệnh; nếu có lỗi thì xử lý lỗi trước khi tiếp tục.

```bash
make -f app/Makefile.demo
make -f app/Makefile.demo run
make -f app/Makefile.demo test
```

Executable là `build/tv06/hc128_demo.exe` trên Windows và
`build/tv06/hc128_demo` trên Linux. Có thể chạy trực tiếp trên Windows:

```bash
./build/tv06/hc128_demo.exe
```

Dùng đúng `-f app/Makefile.demo`: Makefile ở gốc hiện vẫn có target khung,
không phải đường build/kiểm thử đã được xác nhận cho demo này. Không cần dùng
`make clean` ở gốc để chạy demo, vì target đó còn xóa cả thư mục `results`.

## Đầu vào và đầu ra

Key và IV đều là mảng 16 byte toàn `00`, tương ứng vector 1 trong Appendix A
của tác giả. Đây là dữ liệu kiểm thử công khai. Chương trình xuất một dòng gồm
128 ký tự hex chữ thường, tương ứng 64 byte, rồi xuống dòng:

```text
82001573a003fd3b7fd72ffb0eaf63aac62f12deb629dca72785a66268ec758b1edb36900560898178e0ad009abf1f491330dc1c246e3d6cb264f6900271d59c
```

Expected được lấy từ fixture của TV07
`tests/vectors/hc128-kat-001-zero-key-zero-iv.txt`, đối chiếu với Appendix A
trang in 9. Không lấy output của demo để tạo expected.

Mã thoát: `0` khi hoàn tất, `3` nếu ghi stdout thất bại, `4` nếu init hoặc sinh
keystream trả lỗi. Thông báo lỗi nằm ở stderr. Demo chưa nhận tham số dòng lệnh.

## Giải thích app/main.c

1. `key[16] = {0}` và `iv[16] = {0}` đặt toàn bộ 16 byte của mỗi mảng bằng 0.
2. `hc128_ctx ctx` chứa trạng thái được thư viện quản lý; demo không tự sửa P/Q.
3. `hc128_init(&ctx, key, iv)` chỉ được gọi một lần. Nếu hàm báo lỗi, dừng ngay.
4. `hc128_keystream(&ctx, output, sizeof(output))` lấy đúng 64 byte liên tiếp.
5. `print_hex` dùng `%02x` để mỗi byte luôn có hai ký tự, ví dụ `00`, `0a`, `ff`.
   Cast sang `unsigned int` khớp kiểu đối số của `%x`. `fflush` kiểm tra lỗi ghi
   dữ liệu còn nằm trong bộ đệm trước khi báo thành công.

Đầu ra là **keystream**, chưa phải bản mã của một thông điệp. API XOR và xử lý
file thuộc phần triển khai tiếp theo của nhóm.

## Kết quả kiểm thử ngày 25 tháng 10 năm 2026

Lệnh thường `make -f app/Makefile.demo test` kết thúc với mã 0 trên Linux,
biên dịch bằng C11 với `-Wall -Wextra -Wpedantic -Werror`.

- Stdout demo khớp đủ 64 byte của vector 1, đúng hex chữ thường và một newline.
- Ba vector Appendix A đều khớp qua init và keystream thật, mỗi vector 64 byte.
- Chia `1+3+5+55` byte khớp lấy một lần; init lại xóa byte dư; các trường hợp
  NULL/len=0 được kiểm tra về mã trả về và bảo toàn trạng thái.
- Bộ test streaming hiện có của TV09 đạt 100 ca với seed `0x01282026`.
  Đây là kiểm tra tính nhất quán/chạy xen kẽ context, không phải đối chiếu với
  một implementation độc lập.
- Bộ test core hiện có của TV04 đạt ba vector ngắn, 4096 bước kiểm tra trạng thái
  và vector tổng XOR của dòng 64 MiB. Vector dài là kiểm tổng XOR, không so từng
  byte với một file expected dài 64 MiB.
- Các kiểm tra demo/API/stream và core cũng chạy đạt với AddressSanitizer và
  UndefinedBehaviorSanitizer trên Linux. LeakSanitizer tắt trong môi trường này.

Chi tiết compiler, lệnh và log ở [kết quả TV06](../results/tv06-demo-test.txt).
Chưa chạy trên Windows thật hoặc CI của repository. Kết quả này không xác nhận
toàn bộ dự án hoàn thành: full CLI, XOR/file và differential độc lập chưa thuộc
phạm vi kiểm tra.

Để lặp lại kiểm tra sanitizer trên Linux, dùng thư mục build riêng:

```bash
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 \
make -f app/Makefile.demo BUILD_DIR=build/tv06-sanitize \
  CFLAGS='-std=c11 -Wall -Wextra -Wpedantic -Werror -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer' \
  LDFLAGS='-fsanitize=address,undefined' test

ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 \
make -f tests/Makefile.core sanitize
```

## Review và phần tiếp theo

TV07 review demo/expected; TV02/TV03 review helpers và việc ghép init; TV08
review cách đưa demo vào build chung. TV01 hoặc người được chỉ định quyết định
merge sau review theo quy trình nhóm.

Giai đoạn tiếp theo của TV06 là parser/CLI theo `docs/module-contracts.md`,
bao gồm key/IV hex, tham số độ dài, phát hiện đầu vào sai và mã thoát. Demo hiện
tại hoàn thành phạm vi ưu tiên đã được ghi trong bảng phân công.
