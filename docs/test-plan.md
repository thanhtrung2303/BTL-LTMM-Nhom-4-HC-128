# Kế hoạch kiểm thử HC-128

> Trạng thái ban đầu của mọi case: **NOT RUN**. Repo tại thời điểm lập tài liệu chưa có chương trình C, runner, vector hoặc CI. Bảng dưới là yêu cầu triển khai, không phải kết quả PASS.

## 1. Mục tiêu và nguyên tắc

Kiểm tra độ đúng theo nguồn độc lập, xử lý byte/streaming, giao diện, file, khả năng tái lập và minh chứng Input/output. Phạm vi và phân công giữ theo [README](../README.md); các hành vi chi tiết tham chiếu [module contracts](module-contracts.md).

- KAT là cổng bắt buộc; round-trip không thay được KAT.
- Không sinh expected chuẩn từ implementation đang kiểm thử.
- Seed cố định, oracle cố định phiên bản, lưu đầy đủ ca lỗi.
- PASS chỉ khi đã chạy và đạt; FAIL là sai thực tế; BLOCKED là thiếu điều kiện; NOT RUN là chưa chạy; SKIP không tính PASS.
- Suite trả khác 0 khi có FAIL hoặc test bắt buộc BLOCKED/SKIP. Runner không được báo thành công khi không chạy được case nào.
- Không sửa expected hoặc xóa test chỉ để CI xanh.

## 2. Ma trận case và người chịu trách nhiệm

Mỗi ID là một case hoặc một bộ case tham số hóa; runner phải ghi cả ID và tham số cụ thể khi thất bại.

| ID | Chủ trì / reviewer | Input / thao tác | Expected / tiêu chí |
|---|---|---|---|
| T01-01 | TV01 / TV10 | Clone sạch, GCC và Clang, cờ đã thống nhất | Build không warning; không dùng sản phẩm build cũ |
| T01-02 | TV01 / TV10 | Assertion cố ý sai trên nhánh kiểm tra tạm | Runner và CI khác 0; không đưa lỗi cố ý vào main |
| T01-03 | TV01 / TV10 | Thiếu vector/oracle bắt buộc hoặc lọc không ra test | Không báo toàn suite PASS; lỗi rõ và khác 0 |
| T02-01 | TV02 / TV03 | Word 0, FFFFFFFF, một bit, 12345678; các rotate trong đặc tả | Kết quả tính độc lập, đúng chiều và số đếm |
| T02-02 | TV02 / TV03 | Load bytes 00 01 02 03; store 03020100 | Load=0x03020100; store trả đúng 4 byte đầu |
| T02-03 | TV02 / TV03 | Cộng UINT32_MAX+1; helper rotate 0 nếu hỗ trợ tổng quát | Modulo ra 0; không UB ở dịch bit |
| T03-01 | TV03 / TV04 | Zero key/IV và mẫu bất đối xứng; snapshot mở rộng/nạp/warm-up | Khớp snapshot oracle đã xác minh, có mốc và nguồn |
| T03-02 | TV03 / TV04 | Dùng context 1, 3, 5 byte rồi init lại | Dòng mới giống init sạch, không giữ byte dư |
| T03-03 | TV03 / TV04 | ctx/key/iv NULL từng trường hợp | Mã lỗi; ctx hợp lệ không bị đổi khi key/iv NULL |
| T04-01 | TV04 / TV05 | Sinh ít nhất 8193 byte | Khớp oracle toàn bộ, qua nhiều chu kỳ |
| T04-02 | TV04 / TV05 | Quan sát word 511/512, 1023/1024, đánh số từ 0 | Khớp trước/sau offset byte 2048, 4096; counter đúng |
| T05-01 | TV05 / TV06 | Độ dài 0,1,2,3,4,5,63,64,65 | Khớp keystream chuẩn hoặc oracle; đúng kích thước |
| T05-02 | TV05 / TV06 | Chia 1+3,3+1,5+7, chunk có đoạn 0 và chunk có seed | Bằng xử lý một lần cùng tổng độ dài |
| T05-03 | TV05 / TV06 | XOR tách buffer và in-place, chứa 00/FF | Output bằng nhau và bằng input XOR dòng chuẩn |
| T05-04 | TV05 / TV06 | len=0 với buffer NULL; ctx NULL; len>0 buffer NULL | Theo hợp đồng; lỗi phát hiện được không đổi trạng thái/output |
| T05-05 | TV05 / TV06 | keystream 1 byte → XOR 3 byte → keystream 5 byte | Dùng liên tiếp đúng 9 byte, không bỏ byte dư |
| T05-06 | TV05 / TV06 | Encrypt rồi decrypt bằng context mới | Phục hồi input; chỉ là kiểm thử bổ sung |
| T06-01 | TV06 / TV07 | Key/IV 31,32,33 ký tự; chứa G; hex lẻ; hex rỗng | Đúng định dạng được nhận; input sai bị từ chối |
| T06-02 | TV06 / TV07 | Thiếu/lặp/cờ lạ, mode sai, trộn hex/file | Lỗi usage, stderr; không tạo output |
| T06-03 | TV06 / TV07 | --bytes 0, âm, có hậu tố, vượt SIZE_MAX | 0 hợp lệ; còn lại sai; không overflow |
| T06-04 | TV06 / TV07 | Lệnh hợp lệ và --help | Exit 0; format stdout đúng; help không tạo file |
| T07-01 | TV07 / TV09 | KAT zero key/IV có nguồn | Khớp từng byte; ít nhất 64 byte |
| T07-02 | TV07 / TV09 | Tối thiểu 2 cặp key–IV khác có nguồn, ưu tiên một bit/bất đối xứng | Tổng ít nhất 3 cặp độc lập, mỗi cặp ít nhất 64 byte |
| T07-03 | TV07 / TV09 | Vector offset xa nếu nguồn có | Bỏ đúng offset_bytes rồi so; không nhầm word/byte |
| T07-04 | TV07 / TV09 | Fixture vector sai độ dài/hex/thiếu trường | Parser từ chối; không tính PASS hoặc so thiếu byte |
| T08-01 | TV08 / TV01 | File 0,1,3,4,5,4095,4096,4097 byte, 1 MiB, chứa 00/FF | Ciphertext cùng độ dài; round-trip byte-for-byte |
| T08-02 | TV08 / TV01 | File mẫu cố định, xử lý chunk và một lần | Ciphertext giống API một lần; so độc lập cho mẫu nhỏ |
| T08-03 | TV08 / TV01 | Input mất; thư mục đích không ghi được | Khác 0; không có file cuối hỏng; dọn file tạm |
| T08-04 | TV08 / TV01 | Input=output, đường dẫn alias, hard link, symlink | Từ chối; input và output cũ nguyên vẹn |
| T08-05 | TV08 / TV01 | Output tồn tại; giả lập lỗi đọc/ghi/flush/close/công bố | Không ghi đè output cũ; khác 0, dọn tài nguyên |
| T09-01 | TV09 / TV02 | Ít nhất 100 ca seed cố định, key/IV khác nhau, len 0..16384 | Khớp implementation độc lập mọi byte |
| T09-02 | TV09 / TV02 | Xen kẽ hai context A/B, chunk khác nhau | Mỗi dòng bằng chạy riêng cùng key/IV |
| T09-03 | TV09 / TV02 | Chạy suite dưới ASan/UBSan | Không lỗi bộ nhớ hoặc UB trên các ca đã chạy |
| T10-01 | TV10 / TV08 | 1 KiB,1 MiB,16 MiB; warm-up 1 lần, đo ít nhất 5 lần | Output đã kiểm tra; ghi trung vị, thời gian, MiB/s |
| T10-02 | TV10 / TV08 | Demo từ release: build, KAT, file, lỗi CLI | Chạy lại được; số liệu khớp commit release |

T07-03 là mở rộng có điều kiện nguồn; nếu không có vector offset công bố, ghi N/A có lý do và reviewer xác nhận. T04/T09 vẫn bắt buộc kiểm tra dòng dài. Các ca còn lại bắt buộc; không dùng N/A để bỏ qua chức năng chính.

Không gọi API với buffer quá nhỏ hoặc partial overlap để đòi mã lỗi: đó là vi phạm tiền điều kiện. Dùng sanitizer để phát hiện truy cập sai do implementation trên input hợp lệ.

## 3. Quản lý vector và oracle

TV07 chuẩn bị tests/vectors/README.md, manifest.csv và kat.txt. Định dạng chi tiết parser cần chốt M1, bảo đảm các trường:

| Trường | Ý nghĩa |
|---|---|
| id, source, source_case | Định danh ca, nguồn và định danh trong nguồn |
| key_hex, iv_hex | Mỗi giá trị 32 ký tự hex |
| offset_bytes, length_bytes | Đơn vị byte, số nguyên không âm |
| input_hex | Tùy chọn; thiếu nghĩa là kiểm tra keystream |
| expected_hex | Đúng 2 × length_bytes ký tự hex |

manifest.csv ghi file nguồn, URL, phiên bản/commit, SHA-256 file gốc, quy tắc đổi byte, người nhập và reviewer. SHA-256 bảo vệ tính toàn vẹn bản tải, không tự chứng minh nguồn là đúng.

Nếu nguồn xuất word, chuyển từng word little-endian; không đảo toàn chuỗi. Offset áp dụng cho dòng từ init mới. Input/expected có độ dài nhất quán; case lỗi phải bị từ chối trước so sánh.

TV09 cố định phiên bản implementation độc lập và adapter, ghi giấy phép, cấu hình build, ánh xạ key/IV. Xác minh adapter qua KAT trước khi dùng làm oracle. Không sao chép code nhóm sang oracle. Không gọi toàn bộ dữ liệu differential là vector chính thức eSTREAM.

Đề xuất seed suite: 0x1282026. Phải ghi cả thuật toán PRNG tạo input và thứ tự sinh dữ liệu; chỉ ghi seed là chưa đủ để tái lập trên mọi nền tảng. Lưu key, IV, len, chunk và input của ca thất bại.

## 4. Cách chạy đích

Các lệnh chỉ hoạt động sau khi nhóm hiện thực Makefile và các module:

| Lệnh | Nội dung |
|---|---|
| make | Build C11 với cảnh báo đã thống nhất |
| make test | Toàn bộ chức năng T01–T09 phù hợp môi trường; T01-02 kiểm chứng CI riêng có minh chứng |
| make test-kat | KAT và log Input/Expected/Actual đầy đủ |
| make test-sanitize | Build riêng ASan/UBSan, chạy suite |
| make bench | T10 benchmark; không đặt ngưỡng tốc độ tùy tiện trên CI |
| make clean | Xóa build, giữ vector và results đã lưu |

T01-01 clone sạch và T01-02 làm CI thất bại là kiểm tra hạ tầng có quy trình riêng; không để make test tự clone đệ quy hoặc cố ý thất bại trên mọi lần chạy. CI/release checklist phải lưu minh chứng của chúng.

Test lỗi quyền ghi phải chạy bằng user không có quyền phù hợp; nếu môi trường root bỏ qua quyền, dùng fault injection hoặc cấu hình sandbox kiểm thử. Test I/O fail dùng file tạm trong thư mục test, không tác động file người dùng. Không phụ thuộc ngẫu nhiên vào việc ổ đĩa thật hết dung lượng.

## 5. Log và bằng chứng

Mỗi KAT ghi:

```text
Case: <ID>
Source: <nguồn, phiên bản, mã ca>
Key: <hex công khai>
IV: <hex công khai>
Offset bytes: <n>
Length bytes: <n>
Input: <hex hoặc mô tả keystream>
Expected: <hex độc lập>
Actual: <hex từ chương trình>
Result: <PASS/FAIL>
First mismatch: <offset trong đoạn và offset tuyệt đối, nếu sai>
```

Log tổng ghi commit, OS, compiler, cờ, lệnh, oracle version, seed, thời điểm, số PASS/FAIL/BLOCKED/SKIP/NOT RUN. Kết quả dài lưu file kèm vị trí; báo cáo đưa ít nhất 3 ví dụ Input/output đầy đủ.

Lưu log bằng redirect và kiểm tra ngay exit status; việc tạo log không có nghĩa PASS. results/kat.log và results/environment.txt là đường dẫn đích dự kiến, chưa phải bằng chứng hiện có.

## 6. Cổng kiểm thử và release

- M2: helpers/init/core và KAT đầu tiên khớp.
- M3: streaming, lỗi đối số, CLI và file đạt.
- M4: T01–T09 bắt buộc đạt; không còn thiếu oracle; ASan/UBSan sạch.
- M5: T10 có số đo và demo thật; báo cáo khớp log.
- M6: clone sạch và tái chạy trên release candidate; đủ review của từng chủ trì.

Bug report gồm commit, môi trường, lệnh, input, expected/source, actual, offset sai và ảnh hưởng. Sửa xong bổ sung regression rồi chạy lại lớp liên quan; thay lõi phải chạy KAT, streaming và differential. Benchmark chỉ thực hiện sau khi output được xác minh, ghi CPU/OS/compiler/cờ, cách tính thời gian và việc có bao gồm init hay không.
