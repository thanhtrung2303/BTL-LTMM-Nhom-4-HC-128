# Bàn giao TV04 Đồng Xuân Quang

TV04 đã triển khai cập nhật P/Q, counter, sinh word và warm-up theo hợp đồng
API v1 của nhóm. Bộ kiểm thử độc lập của module đạt ba vector ngắn và vector dài
64 MiB trong Appendix A của tác giả. Đây là kết quả kiểm thử core bằng bộ nạp
trạng thái dành riêng cho test; chưa phải kết quả tích hợp init/stream của nhóm.

## Các file bàn giao

| File | Mục đích |
|---|---|
| `src/hc128_core.c` | Hai hàm `hc128_step_word` và `hc128_warmup` |
| `src/hc128_internal.h` | Khai báo đúng hai hàm nội bộ đã chốt |
| `tests/test_core.c` | KAT ngắn, dài và kiểm tra biên/trạng thái |
| `tests/Makefile.core` | Chạy test TV04 riêng, không phụ thuộc runner khung |
| `docs/tv04-core.md` | Giải thích, hướng dẫn tích hợp và trình bày |
| `results/tv04-core-test.txt` | Lệnh, phiên bản compiler và kết quả thực tế |

Public header `include/hc128.h` không thay đổi. Header nội bộ chưa tồn tại ở main
khi triển khai nên bản bàn giao bổ sung hai khai báo tối thiểu. TV02 tiếp tục sở hữu
header này; nếu TV02 đã có bản mới thì ghép hai khai báo vào bản của TV02, không
chép đè các helpers. Các helper trong core là `static`, chỉ dùng trong file đó.

## Cách chạy trên máy của bạn

Dùng terminal **MSYS2 UCRT64** trong VS Code như README của nhóm, tại thư mục gốc
repo. Giải nén gói bàn giao, chép các file vào đúng thư mục tương ứng trong repo.
Thư mục `include` trong gói chứa bản header hiện tại để có thể chạy test độc lập;
nếu repo của nhóm có header mới hơn thì dùng bản của nhóm và đối chiếu hợp đồng.

```bash
make -f tests/Makefile.core test
```

Kết quả cuối phải là `PASS all TV04 standalone core tests`. Lệnh có mã thoát khác
0 khi biên dịch hoặc kiểm tra thất bại. Không dùng `make test` hiện tại của main
để kết luận core đúng, vì runner ở main chỉ in thông báo khung.

Nếu chưa dùng Make, vẫn có thể chạy trong UCRT64:

```bash
mkdir -p build/tv04
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -O2 -Iinclude -Isrc src/hc128_core.c tests/test_core.c -o build/tv04/test_core.exe
./build/tv04/test_core.exe
```

Linux dùng cùng lệnh Make. Kiểm tra sanitizer trên Linux:

```bash
make -f tests/Makefile.core sanitize
```

Trong môi trường kiểm thử lần này, LeakSanitizer không đọc được `/proc` do giới
hạn môi trường. Lệnh thực tế chạy đạt là:

```bash
ASAN_OPTIONS=detect_leaks=0 make -f tests/Makefile.core sanitize
```

AddressSanitizer và UndefinedBehaviorSanitizer vẫn bật; kiểm tra leak bị tắt.
Core không cấp phát động. Chưa chạy trên máy Windows thật hoặc CI của nhóm.

## Giải thích thuật toán

Mỗi bảng P và Q có 512 phần tử `uint32_t`. Counter chạy từ 0 tới 1023.
`j = counter & 511` chính là counter modulo 512.

| Counter trước bước | Bảng được cập nhật | Bảng tra cứu h |
|---|---|---|
| 0 đến 511 | P[j] | Q |
| 512 đến 1023 | Q[j] | P |

Ký hiệu ROR là xoay phải, ROL là xoay trái trên word 32 bit. Khác với dịch bit,
xoay đưa các bit bị đẩy ra trở lại đầu bên kia. Công thức trong mục 2.1 của đặc tả:

```text
g1(x,y,z) = (ROR(x,10) XOR ROR(z,23)) + ROR(y,8)
g2(x,y,z) = (ROL(x,10) XOR ROL(z,23)) + ROL(y,8)
h1(x) = Q[x & 255] + Q[256 + ((x >> 16) & 255)]
h2(x) = P[x & 255] + P[256 + ((x >> 16) & 255)]
```

Phép cộng word lấy modulo 2^32. Các phép xoay chỉ nhận số đếm cố định 8, 10, 23,
không có dịch 32 bit. `h` trong code dùng chung công thức; truyền Q cho h1 và P cho
h2. Hai byte được lấy là byte thứ 0 và thứ 2, tính từ byte thấp nhất.

Trong nhánh P, cộng g1 vào P[j] trước rồi trả về `P[j] XOR h1(P[j-12])`.
Nhánh Q tương tự với g2/h2. Các chỉ số j-3, j-10, j-12, j-511 đều lấy modulo
512. Ví dụ j=0 thì j-3 thành 509. j-511 tương đương j+1 modulo 512.
Cuối mỗi bước: `counter = (counter + 1) & 1023`.

Khi counter=511, vẫn cập nhật P[511], sau đó counter thành 512. Bước tiếp theo
mới cập nhật Q[0]. Khi counter=1023, cập nhật Q[511], sau đó counter về 0.

### Vì sao warm-up phải ghi phản hồi

Sau khi TV03 nạp P/Q từ W và đặt counter=0, gọi `hc128_warmup` đúng một lần.
Hàm chạy 512 bước với P, ghi từng word kết quả vào P[j] ngay sau mỗi bước;
sau đó làm tương tự 512 bước với Q. Như vậy các bước sau dùng trạng thái đã phản
hồi. Kết thúc counter tự quay về 0. Không thể thay bằng việc gọi sinh word 1024
lần và bỏ kết quả, vì trạng thái giữ lại trong bảng sẽ khác.

Hai hàm không sửa `byte_buffer`/`byte_pos`, không in, không đọc file, không cấp phát
động. TV03 đặt byte_buffer về 0 và byte_pos=4 sau warm-up. TV05 chịu trách nhiệm
chuyển word thành byte little-endian và giữ byte dư. Không gọi xen kẽ core trực
tiếp với API byte trên cùng context, vì sẽ bỏ qua byte dư của lớp stream.

## Ghép với các thành viên

- **TV02:** review phép xoay, số học unsigned và ghép header nội bộ.
- **TV03:** include header nội bộ; sau khi nạp P/Q và counter=0, gọi warm-up.
- **TV05:** gọi step_word khi hết byte dư; không warm-up lại trong quá trình sinh.
- **TV06:** demo gọi API public sau khi init/stream đã có, không tự truy cập P/Q.
- **TV08:** thêm `src/hc128_core.c` vào danh sách nguồn build của chương trình.
  Bộ test độc lập có `main` riêng, không ghép chung với `app/main.c` hoặc runner khác.
- **TV07:** chạy lại KAT qua public API khi ghép xong init và stream.

`prepare()` trong test là bộ nạp W tối thiểu để test core trước khi module TV03
có mặt. Nó không triển khai `hc128_init`, chỉ hỗ trợ ba mẫu key/IV trong Appendix A.
Không chép helper này vào chương trình chính thay cho công việc của TV03.

## Nguồn và kiểm định

Đặc tả: Hongjun Wu, *The Stream Cipher HC-128*, mục 2.1 trang 2 (công thức),
2.2 trang 3 (warm-up), 2.3 trang 4 (sinh word), Appendix A trang 9–10 (expected).
Trang ở đây là số in trên tài liệu.

- URL được TV07 ghi: https://www3.ntu.edu.sg/home/wuhj/research/hc/hc128.pdf
- URL eSTREAM: https://www.ecrypt.eu.org/stream/p3ciphers/hc/hc128_p3.pdf
- Bản thực tế đã đọc lấy từ `tools/reference/hc128-reference.pdf` trên nhánh TV07,
  commit `85fdee7be5b73a620e5f2e5b4645f85f96b68837` của repo nhóm.
- SHA-256 PDF: `cfa5d5ff456691468f3e31d68e0c6a76181641bda352b22a107372e765d904a0`.
- Repo nền: `bb866da` (main tại lúc tải); ngày kiểm tra: 20/10/2026.

Expected trong test chép theo word 32 bit từ tài liệu tác giả, không sinh từ core.
Ví dụ word đầu vector 1 là 0x73150082; nếu in byte little-endian sẽ là `82001573`.
Ba vector ngắn mỗi vector 16 word. Vector dài dùng key/IV toàn 0, XOR tích lũy
16 vị trí word qua 2^20 khối. Tổng cộng 2^24 word = 64 MiB, counter quay 16.384
vòng. Đây là phép kiểm tổng XOR theo đặc tả, không phải so từng byte của 64 MiB.

Kiểm tra bổ sung 4.096 bước xác nhận chỉ đúng phần tử của đúng bảng có thể thay
đổi, counter đúng ở mọi bước và bộ đệm byte còn nguyên. KAT dài giúp kiểm tra
nhánh Q, nhiều lần chuyển bảng và vòng lặp counter mà vector ngắn chưa chạm tới.
Các kết quả này không thay cho review độc lập hoặc KAT tích hợp của nhóm.

## Đoạn trình bày khoảng một phút

“Em phụ trách lõi HC-128. Trạng thái có hai bảng P và Q, mỗi bảng 512 word 32 bit.
Counter chọn bảng: 512 bước đầu cập nhật P, 512 bước sau cập nhật Q. Mỗi bước
cộng hàm g vào một phần tử, rồi XOR với hàm h tra bảng còn lại để sinh word.
Counter tăng modulo 1024, còn chỉ số trong bảng lấy modulo 512. Trong warm-up,
word đầu ra được ghi trở lại bảng trong 1024 bước để hoàn thiện khởi tạo.
Em đã kiểm tra ba vector ngắn và vector tổng XOR của dòng 64 MiB theo tài liệu
tác giả, cùng test chuyển bảng và counter. Phần chuyển word thành byte do TV05
phụ trách; nhóm cần chạy lại KAT qua API chung sau khi tích hợp.”

## Đưa phần việc lên nhánh của bạn

Nếu đã có repo trên máy, chạy `git status` và xử lý các thay đổi đang làm trước
khi chuyển nhánh. Từ bản main sạch, tạo nhánh trước khi chép file bàn giao:

```bash
git switch main
git pull --ff-only origin main
git switch -c feat/TV04-core
```

Nếu nhánh đã có thì dùng `git switch feat/TV04-core`. Chép các file bàn giao vào,
kiểm tra nội dung, chạy test rồi lưu commit:

```bash
make -f tests/Makefile.core test
git add src/hc128_core.c src/hc128_internal.h tests/test_core.c tests/Makefile.core docs/tv04-core.md results/tv04-core-test.txt
git diff --cached
git commit -m "feat(core): implement HC-128 word generation and warmup"
git push -u origin feat/TV04-core
```

Mở Pull Request từ nhánh trên vào main, nhờ TV02/TV03 và người tích hợp review.
Log đi kèm là kết quả Linux trong lần bàn giao này; nếu chạy trên máy của bạn,
ghi riêng kết quả mới và môi trường thực tế. Chưa có commit/push/PR nào được tạo
trên GitHub trong lần bàn giao này.
