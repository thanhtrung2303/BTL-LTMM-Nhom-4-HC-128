# Quy ước phát triển dự án HC-128

> Nhóm 4 — Lý thuyết mật mã. Tài liệu hướng dẫn triển khai bằng C, phát triển từ [README](../README.md). Chưa có mã nguồn hoặc kết quả kiểm thử được xác nhận tại thời điểm lập tài liệu.

## 1. Bộ tài liệu và phạm vi

| Tài liệu | Nội dung |
|---|---|
| [Team plan](team-plan.md) | Người phụ trách, review, mốc bàn giao |
| [Spec notes](spec-notes.md) | Tham số, luồng thuật toán, điểm cần kiểm tra đặc tả |
| [Module contracts](module-contracts.md) | API, trạng thái, giao tiếp giữa các module |
| [Test plan](test-plan.md) | Ca kiểm thử, oracle, minh chứng và nghiệm thu |

README là nguồn phân công và phạm vi tổng thể. Các quyết định chi tiết mới trong docs là đề xuất cần review tại M1; không coi chúng là API đã tồn tại. Nếu thay đổi phạm vi hoặc API, sửa các tài liệu liên quan trong cùng PR. README dự kiến thêm design.md; trong giai đoạn này, spec-notes.md và module-contracts.md cung cấp nội dung thiết kế tương ứng.

## 2. C và tổ chức mã

- C11; môi trường chuẩn Ubuntu/WSL; GCC và Clang.
- Cờ tối thiểu: `-std=c11 -Wall -Wextra -Wpedantic`. Không bỏ cảnh báo bằng cách tắt cờ toàn dự án.
- Dùng `uint32_t` cho word, `uint8_t` cho byte, `size_t` cho độ dài bộ nhớ. Số học thuật toán theo modulo 2^32.
- Tên hàm/biến/file dùng snake_case; API công khai có tiền tố hc128_; macro dùng HC128_ và chữ hoa.
- Thụt 4 dấu cách, không tab; UTF-8, LF, newline cuối file; tránh dòng quá 100 ký tự khi hợp lý.
- Header có include guard, tự include kiểu dữ liệu mà nó sử dụng. Helpers dùng nội bộ không xuất qua header công khai.
- Hàm nội bộ chỉ dùng trong một file phải static. Không dùng trạng thái toàn cục thay đổi được.
- Load/store little-endian tường minh; không ép con trỏ byte thành con trỏ uint32_t. Cast sang kiểu unsigned phù hợp trước dịch bit.
- Không dịch 32 bit trên word 32 bit. Nếu helper rotate nhận số đếm tổng quát, chuẩn hóa số đếm và xử lý riêng 0.
- Lõi không malloc, printf, đọc file hay parse hex. CLI/file chịu trách nhiệm cấp buffer, thông báo và I/O.
- Comment giải thích lý do, chỉ số thuật toán, nguồn công thức; không chỉ diễn giải lại câu lệnh.
- Mỗi lỗi được sửa cần ca regression có khả năng phát hiện lỗi cũ.

## 3. Quy ước API và lỗi

Tuân thủ [hợp đồng module](module-contracts.md). Kiểm tra đối số có thể kiểm tra trước khi thay đổi context/output. Không hứa kiểm tra được dung lượng vùng nhớ hoặc con trỏ treo. Partial overlap là vi phạm tiền điều kiện, không phải trường hợp bắt buộc phát hiện khi chạy.

Không ghi khóa thật vào log. Key/IV trong KAT là dữ liệu công khai có chủ đích. Không tái dùng cùng cặp key–IV cho các bản tin độc lập ngoài kiểm thử.

## 4. Quy trình GitHub

1. Tạo Issue ghi mục tiêu, file, chủ trì, reviewer, test ID, nghiệm thu và hạn thực tế.
2. Đồng bộ main và tạo nhánh riêng: `feat/TV03-init`, `test/TV07-kat`, `docs/TV10-slides`, `fix/TV05-byte-buffer`.
3. Commit theo mẫu `type(scope): mô tả`; type gồm feat, fix, test, docs, build, ci, refactor.
4. Chạy test liên quan; thay lõi/API phải chạy KAT và streaming.
5. Mở PR liên kết Issue; ghi lệnh và kết quả thật, phần chưa chạy và lý do.
6. Reviewer đọc diff, checkout nhánh, kiểm tra nguồn expected và chạy lại test liên quan.
7. Chỉ merge khi có approval độc lập, CI đạt và góp ý bắt buộc đã xử lý. TV01 hoặc người được chỉ định merge.
8. Đồng bộ main sau merge; không force-push nhánh chung.

Mẫu nội dung PR:

```markdown
## Mục tiêu
Closes #<issue>

## Thay đổi
- Module/tài liệu và hành vi thay đổi.

## Kiểm tra
- Lệnh:
- Kết quả thực tế:
- Chưa chạy / giới hạn:

## Review
- Người review:
- Test ID và tài liệu bị ảnh hưởng:
```

Không tự approve. Với PR chỉ sửa tài liệu, kiểm tra liên kết, phân công, API, test ID và độ chính xác kỹ thuật; ghi rõ không chạy test C nếu chưa có code. Không tạo kết quả PASS giả để đáp ứng mẫu.

## 5. Build và kiểm thử

Các target đích cần triển khai: make, make test, make test-kat, make test-sanitize, make bench, make clean. Chi tiết trong test-plan.md.

CI mỗi PR: GCC/Clang, suite chức năng và ít nhất một cấu hình ASan/UBSan. Build sanitizer ở thư mục riêng, không dùng lại object tối ưu. Thiếu oracle/dependency bắt buộc phải báo BLOCKED và làm job thất bại, không âm thầm bỏ qua.

## 6. Quản lý dữ liệu và bàn giao

- Không commit build/, object, executable, khóa thật hoặc dữ liệu riêng tư.
- Commit vector công khai cùng nguồn, phiên bản, checksum và giấy phép nếu có.
- results/ chỉ lưu minh chứng chọn lọc có commit, compiler, cờ build, lệnh chạy, seed và thời điểm.
- Expected chỉ thay đổi khi có bằng chứng nguồn và review độc lập; không lấy output đang sai làm chuẩn.
- Tên file tài liệu dùng chữ thường, nối bằng dấu gạch ngang.
- Báo cáo/slide phải phân biệt thiết kế dự kiến, kết quả thực nghiệm và giới hạn kết luận.
- Trước release v1.0.0, clone sạch và chạy đúng các lệnh công bố; source, log, báo cáo và slide phải cùng phiên bản.

Lưu ý đường dẫn: clone mặc định tạo thư mục BTL-LTMM-Nhom-4-HC-128, nên dùng `cd BTL-LTMM-Nhom-4-HC-128`. Tên hc128-project trong cây README là tên minh họa; chỉ dùng nếu đã đặt tên thư mục clone tương ứng.
