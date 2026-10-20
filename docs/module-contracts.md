# Hợp đồng giữa các module

> API/context v1 được chốt ngày 20/10/2026 theo phân công mới của TV01. Giữ nguyên public API trong include/hc128.h. TV05 triển khai lớp byte; TV08 tích hợp build/runner. Đây là hợp đồng triển khai, không phải xác nhận thuật toán hoặc KAT đã đạt.

## 1. Phân lớp và quyền sở hữu

| Module | Chủ trì | Nhận / trả | Không được làm |
|---|---|---|---|
| include/hc128.h | TV01, phối hợp TV05 | Kiểu context, public API | Phụ thuộc CLI/I/O |
| src/hc128_internal.h | TV02 | Helpers, khai báo bước nội bộ | Xuất API ứng dụng tùy tiện |
| src/hc128_init.c | TV03 | Key/IV → context sẵn sàng | Xuất byte warm-up |
| src/hc128_core.c | TV04 | Context → word kế tiếp | Quản lý hex/file |
| src/hc128_stream.c | TV05 | Word → byte; input XOR dòng khóa | Tự init lại giữa các chunk |
| app/main.c, hex.* | TV06 | argv/hex → dữ liệu đã xác thực | Truy cập P/Q trực tiếp |
| app/file_io.* | TV08 | File → chunk → API → file | Nạp cả file lớn bắt buộc vào RAM |
| tests/, tools/reference/ | TV01/07/09 | Case/oracle → so sánh/log | Dùng lõi nhóm tạo expected chuẩn |
| bench/ | TV10 | Cấu hình đo → thời gian, MiB/s | Thay đổi hành vi lõi để đạt số đo |

Luồng: CLI kiểm tra toàn bộ tham số → init → xử lý byte/chunk → xuất kết quả. Unit/KAT gọi thẳng public API, test CLI/file kiểm tra thêm lớp ứng dụng.

## 2. Context và vòng đời

Layout đã chốt, khớp include/hc128.h:

| Trường | Kiểu / ý nghĩa |
|---|---|
| p, q | Hai mảng uint32_t[512] |
| counter | uint32_t, giá trị 0..1023 |
| byte_buffer | uint8_t[4], byte little-endian của word mới nhất |
| byte_pos | size_t, vị trí byte kế tiếp 0..4; 4 nghĩa là buffer rỗng |

Caller cấp context và gọi init trước mọi thao tác. Không cần cấp phát động trong lõi. Mỗi context chỉ xử lý một dòng, không gọi đồng thời trên cùng context; các context khác nhau độc lập. Caller không sửa trường nội bộ và không dựa vào layout để lưu định dạng file.

Sau init thành công: counter=0, byte_pos=4 và byte_buffer được đặt về 0. Init lại thay thế toàn bộ trạng thái cũ và xóa ý nghĩa byte dư. Không hứa tự phát hiện context chưa init hay bị hỏng.

## 3. Public API v1 đã chốt

Đây là chữ ký hợp đồng, không phải mã triển khai:

```c
int hc128_init(hc128_ctx *ctx, const uint8_t key[16],
               const uint8_t iv[16]);
int hc128_keystream(hc128_ctx *ctx, uint8_t *out, size_t len);
int hc128_xor(hc128_ctx *ctx, const uint8_t *in,
              uint8_t *out, size_t len);
```

Tên kiểu hc128_ctx và layout cần được TV01 đưa vào include/hc128.h. Quy ước mã lỗi đã chốt: HC128_OK=0, HC128_ERR_ARGUMENT=1.

| API | Tiền điều kiện | Hậu điều kiện thành công |
|---|---|---|
| init | ctx/key/iv khác NULL; key và iv có ít nhất 16 byte hợp lệ, không alias vùng ctx | Context sẵn sàng; reset dòng và byte dư |
| keystream | ctx đã init; len>0 thì out hợp lệ đủ len byte | Ghi đúng len byte liên tiếp; dòng tiến đúng len byte ở mức public API |
| xor | ctx đã init; len>0 thì in/out hợp lệ đủ len byte | out[i]=in[i] XOR byte keystream tương ứng; tiến đúng len byte |

Quy tắc chung:

- ctx luôn phải khác NULL, kể cả len=0.
- len=0 cho phép buffer NULL; trả thành công nếu ctx hợp lệ; không ghi output/tiến trạng thái.
- len>0 với buffer NULL: trả lỗi trước khi thay đổi context/output.
- API không nhận key_len/iv_len nên không kiểm tra được vùng key/iv ngắn. Caller đảm bảo 16 byte; CLI kiểm tra 32 ký tự hex.
- Hỗ trợ in==out. Input và output được phép tách rời hoàn toàn; chồng lấn một phần không được hỗ trợ.
- Buffer/key/iv không được chồng lên context. Không yêu cầu API phát hiện mọi alias sai.
- Chỉ các lỗi đối số có thể phát hiện mới có cam kết “không thay đổi”. Không đảm bảo hành vi khi caller đưa con trỏ không hợp lệ, thiếu dung lượng hoặc context chưa init.
- Không gọi strlen trên dữ liệu nhị phân.
- Không có API seek; muốn offset n thì init mới và tiêu thụ đúng n byte.

## 4. Hợp đồng init ↔ core ↔ stream

Hai hàm nội bộ được khai báo trong src/hc128_internal.h và triển khai bởi TV04 trong src/hc128_core.c:

```c
uint32_t hc128_step_word(hc128_ctx *ctx);
void hc128_warmup(hc128_ctx *ctx);
```

- hc128_step_word: tiền điều kiện ctx đã init thành công; sinh word kế tiếp theo đặc tả, cập nhật P/Q và tăng counter modulo 1024. Không đọc/sửa byte_buffer hoặc byte_pos. Chỉ stream gọi trong đường chạy ứng dụng; unit test có thể gọi trực tiếp với context riêng, không trộn gọi core và public API trên cùng context.
- hc128_warmup: chỉ TV03 gọi sau khi mở rộng khóa/IV và nạp P/Q, với counter=0. TV04 thực hiện đầy đủ 1024 bước phản hồi khởi tạo theo đặc tả HC-128, kết thúc counter=0. Không xuất keystream, không xử lý byte buffer. Không thay bằng vòng lặp bỏ output của hc128_step_word: phản hồi khởi tạo phải ghi lại P/Q theo đặc tả.
- TV03 chịu trách nhiệm load key/IV little-endian, mở rộng W và nạp P/Q; gọi hc128_warmup đúng một lần, đặt byte_buffer về 0, byte_pos=4 rồi trả HC128_OK.
- Hai hàm nội bộ không kiểm tra NULL, không trả mã lỗi; caller bảo đảm tiền điều kiện.
- TV02 giữ khai báo chung và helpers trong header nội bộ; TV03/04 không tự khai báo hàm khác chữ ký ở file riêng.

Lớp stream:

1. Nếu còn byte dư, lấy byte đó trước.
2. Nếu hết byte dư, gọi core đúng một lần, store little-endian vào buffer và đặt byte_pos=0.
3. Trả hoặc XOR số byte cần dùng; tăng byte_pos tương ứng.
4. Không bỏ byte dư ở cuối lời gọi.

keystream và xor dùng chung cơ chế tiêu thụ byte. Ví dụ lấy 1 byte bằng keystream rồi XOR 3 byte phải dùng đúng 4 byte đầu cùng dòng, không nhảy sang word tiếp theo. Counter phản ánh số bước word nội bộ, không phải số lời gọi public API.

## 5. Hợp đồng CLI và hex

Giao diện đích theo README:

- keystream --key HEX32 --iv HEX32 --bytes N
- encrypt/decrypt --key HEX32 --iv HEX32 --hex DATA
- encrypt/decrypt --key HEX32 --iv HEX32 --in INPUT --out OUTPUT

Đề xuất chi tiết cần review tại M1:

| Mục | Hành vi |
|---|---|
| Key/IV | Đúng 32 ký tự hex; chấp nhận a–f/A–F; không có tiền tố 0x |
| Data hex | Số ký tự chẵn; cho phép chuỗi rỗng; không bỏ qua khoảng trắng ở giữa |
| --bytes | Số thập phân không dấu, cho phép 0; kiểm tra overflow trước chuyển size_t |
| Mode | Chỉ một mode; không trộn --hex với --in/--out |
| Cờ | Từ chối cờ lạ, cờ lặp, thiếu giá trị, đối số thừa |
| Output hex | Chữ thường, hai ký tự/byte, không khoảng trắng, một newline cuối |
| Thông báo | Kết quả ở stdout; lỗi ở stderr; không in key trong lỗi nhập liệu |
| --help | Hướng dẫn và exit 0, không tạo output file |

Parse/validate hết tham số trước init hoặc tạo file đầu ra. Bộ hex phải kiểm tra dung lượng đích và hex hợp lệ trước khi ghi; chữ ký cụ thể của helper do TV06 chốt trong app/hex.h. CLI xử lý output lớn theo chunk, kiểm tra lỗi ghi stdout.

Mã thoát đề xuất: 0 thành công; 2 sai cách dùng/định dạng; 3 lỗi I/O; 4 lỗi cấp phát/lỗi nội bộ. Đây là mã CLI, không đồng nhất với mã public API. Không dùng “sai key về giá trị” làm lỗi: mã hóa thô không xác thực.

## 6. Hợp đồng file

- File input/output ở chế độ nhị phân (`rb`/`wb` khi dùng stdio), output chỉ chứa ciphertext thô, không thêm key/IV/header. Trên Windows không được để chuyển đổi CRLF hoặc xử lý byte 0x1A như EOF do chế độ text.
- Dùng một context cho toàn bộ file, chunk mặc định đề xuất 4096 byte; chunk cuối xử lý đúng số byte đã đọc.
- Phân biệt EOF bình thường với lỗi đọc; kiểm tra ghi thiếu, lỗi flush/close và lỗi đổi tên.
- Từ chối cùng file trước khi mở ghi phá hủy dữ liệu: trên Windows so định danh file từ handle (volume + file ID hoặc cơ chế tương đương), gồm hard link/symlink alias. Trên Linux dùng thiết bị/inode. Không chỉ so chuỗi đường dẫn; Windows có khác biệt chữ hoa/thường và dấu phân cách.
- Đề xuất mặc định từ chối output đã tồn tại; chưa thêm --force vào phạm vi.
- Ghi qua file tạm cùng thư mục đích; chỉ công bố file cuối khi toàn bộ xử lý/đóng file thành công. Công bố phải không ghi đè file xuất hiện đồng thời; không chỉ dựa vào kiểm tra tồn tại trước đó.
- Khi lỗi: đóng tài nguyên, dọn file tạm đã tạo, trả mã khác 0; bảo toàn input và output cũ.
- TV08 triển khai lớp I/O phù hợp Windows UCRT64, đồng thời có nhánh Linux để CI chạy. Cô lập API Windows/POSIX trong app/file_io.* với điều kiện biên dịch; không đưa phụ thuộc hệ điều hành vào include/hc128.h hoặc src/. Ghi rõ cơ chế tạo tạm độc quyền, công bố không ghi đè và giới hạn chống race trên mỗi nền tảng.
- File rỗng tạo output rỗng thành công. Không coi nội dung có byte 00 là kết thúc dữ liệu.

## 7. Thay đổi hợp đồng và nghiệm thu

Mọi thay đổi: Issue nêu lý do → review bởi chủ trì và caller → cập nhật header/docs/test → tích hợp đồng bộ. TV01/05 duyệt API; TV06/08 xác nhận CLI/file; TV07/09 xác nhận khả năng kiểm thử.

Nghiệm thu hợp đồng: T03 reset, T04 biên word, T05 len=0/byte dư/in-place/trộn API, T06 validation/mã thoát, T08 bảo toàn file, T09 context độc lập. Xem [test plan](test-plan.md).

## 8. Phạm vi demo 21/10 và tiêu chí kiểm tra giao diện

- Chốt cả ba public API ngay; trước buổi kiểm tra ưu tiên init + keystream, XOR được triển khai giai đoạn sau theo phân công TV05. Có khai báo không có nghĩa hàm đã được triển khai. Demo/KAT chưa gọi XOR khi chưa có định nghĩa; không viết stub trả thành công giả.
- TV06: demo dùng key/IV cố định dưới dạng mảng 16 byte, init một lần rồi lấy 64 byte và in hex. Chưa yêu cầu parser/CLI đầy đủ.
- TV07: KAT dùng expected độc lập; không gọi core trực tiếp để thay thế public API. Báo byte sai đầu tiên, expected/actual và trả mã thất bại.
- TV08: build các module thật và runner; thiếu module hoặc test phải báo chưa hoàn thành, không in PASS giả.
- TV01: kiểm tra header/hợp đồng thống nhất, chốt commit demo và trạng thái thực tế. CI để giai đoạn sau.
- Khi bộ test sẵn sàng: so 64 byte một lần với các chunk 1+3+5+55 trên context khởi tạo độc lập; kiểm tra init lại tái lập output, len=0 không tiến trạng thái, NULL có thể phát hiện trả lỗi trước khi thay đổi. Trộn keystream/XOR và in-place được kiểm tra khi XOR triển khai.
- Tên và kiểu helper toán học còn lại do TV02/03/04 thống nhất trong header nội bộ; không mở rộng public API v1 nếu chưa qua quy trình mục 7.
