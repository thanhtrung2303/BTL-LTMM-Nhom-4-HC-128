# Kế hoạch nhóm và phân công

> Giữ nguyên mã TV, họ tên, MSSV và vai trò trong [README](../README.md). Đây là kế hoạch, không phải xác nhận các thành viên đã hoàn thành công việc.

## 1. Thành viên, sản phẩm và review

| Mã | Họ tên | MSSV | Phụ trách / sản phẩm chính | Test | Reviewer |
|---|---|---|---|---|---|
| TV01 | Nguyễn Thành Trung | 20233680 | include/hc128.h cùng TV05; Makefile; CI; test_main.c; tích hợp | T01 | TV10 |
| TV02 | Tăng Tiến Dũng | 20233350 | Đặc tả, helpers trong src/hc128_internal.h; spec-notes.md | T02 | TV03 |
| TV03 | Phạm Quốc Cường | 20233295 | src/hc128_init.c; mở rộng khóa/IV, khởi tạo | T03 | TV04 |
| TV04 | Đồng Xuân Quang | 20233603 | src/hc128_core.c; cập nhật P/Q, sinh word | T04 | TV05 |
| TV05 | Vũ Khắc Hùng | 20233428 | src/hc128_stream.c; byte dư, XOR, API | T05 | TV06 |
| TV06 | Đặng Quốc Anh | 20233240 | app/main.c, app/hex.c, app/hex.h; CLI | T06 | TV07 |
| TV07 | Nguyễn Anh Tú | 20233691 | tests/vectors/; test_kat.c; nguồn và manifest | T07 | TV09 |
| TV08 | Nguyễn Tuấn Việt | 20233713 | app/file_io.c, app/file_io.h; fixture, test file | T08 | TV01 |
| TV09 | Trần Thái Sơn | 20233625 | tools/reference/; test_differential.c; sanitizer | T09 | TV02 |
| TV10 | Trần Văn Vinh | 20223829 | bench/bench_hc128.c; tổng hợp báo cáo/slide, demo | T10 | TV08 |

TV01 chủ trì conventions.md và team-plan.md; TV02 chủ trì spec-notes.md; TV01/TV05 đồng thuận module-contracts.md; TV07 chủ trì test-plan.md với TV09 kiểm tra oracle. Mỗi file mã nguồn có một người chủ trì; thay giao diện dùng chung phải có ý kiến người phụ thuộc.

## 2. Thứ tự triển khai

| Mốc | Công việc | Phụ thuộc | Điều kiện hoàn tất |
|---|---|---|---|
| M0 | TV01 tổ chức Issue, reviewer, phạm vi, deadline | Không | Đủ 10 nhiệm vụ có người nhận |
| M1 | TV01/02/05 chốt API; TV06 chốt CLI; TV07/09 xác minh nguồn | M0 | Hợp đồng được review; có expected độc lập đầu tiên |
| M2 | TV01 dựng build/runner; TV02 helpers; TV03 init; TV04 core | M1 | Unit liên quan đạt và ít nhất một KAT end-to-end khớp |
| M3 | TV05 byte/XOR; TV06 CLI; TV08 file | M2 để tích hợp | Streaming, độ dài lẻ, input lỗi và file đạt |
| M4 | TV07 hoàn thiện KAT; TV09 differential/sanitizer | M3 | T01–T09 đạt; không còn BLOCKED/SKIP bắt buộc |
| M5 | TV10 benchmark và demo; cả nhóm hoàn thiện nội dung | M4 | T10 có số đo thật; tài liệu khớp release candidate |
| M6 | TV01 clone sạch, cả nhóm tổng duyệt | M5 | Checklist đủ; tag v1.0.0 trỏ đúng commit |

Chưa có ngày nộp được cung cấp: TV01 điền hạn cụ thể trên Issue trước M0, không tự coi số mốc là số tuần. TV07/09 bắt đầu tìm nguồn ngay M1. TV06/08 có thể xây parser và I/O với stub theo hợp đồng, nhưng kết quả với stub không tính nghiệm thu thuật toán.

## 3. Phụ thuộc và phối hợp

- TV02 bàn giao helpers và cách đánh chỉ số cho TV03/04.
- TV03 và TV04 thống nhất cách cập nhật khi khởi tạo trước khi dùng chung hàm bước; không gộp warm-up với bỏ keystream thông thường.
- TV04 bàn giao word generator; TV05 chịu trách nhiệm toàn bộ chuyển byte và giữ byte dư.
- TV05 bàn giao API byte cho TV06/08; CLI/file không được tự thao tác P/Q.
- TV07 cung cấp vector độc lập cho TV03–05; TV09 đối chiếu dài và ca ngẫu nhiên có seed.
- TV10 nhận phần chuyên môn từ từng tác giả; không viết thay hoặc xác nhận thay kết quả chưa chạy.

Khi bị chặn, ghi Issue: phụ thuộc nào, người cần hỗ trợ, phần đã làm, bằng chứng, bước tiếp theo. Thay API phải sửa header, module-contracts.md và test/caller bị ảnh hưởng trong cùng PR hoặc chuỗi PR có thứ tự tích hợp rõ.

## 4. Review chéo

Reviewer chính theo bảng mục 1; việc “TV01 được TV10 review” đồng thời có nghĩa TV10 phải thực hiện review T01. Reviewer ghi lệnh đã chạy và nhận xét kỹ thuật, không chỉ bấm approve.

| Phần | Trọng tâm review |
|---|---|
| Helpers / đặc tả | Kiểu unsigned, rotate, endian, nguồn |
| Init | Chỉ số mở rộng, nạp bảng, phản hồi khởi tạo, reset |
| Core | Chuyển P/Q, counter, thứ tự cập nhật |
| Stream | Byte dư, trộn keystream/XOR, in-place, len=0 |
| CLI/file | Validation trước I/O, mã lỗi, không mất input |
| KAT/differential | Expected độc lập, offset, seed, provenance |
| Build/tài liệu/benchmark | Tái lập, mã thoát, phương pháp đo, kết quả thật |

Nếu reviewer vắng, TV01 chỉ định người thay và ghi trong PR. Không tự review để đáp ứng approval bắt buộc.

## 5. Báo cáo, slide, trình bày

| TV | Nội dung báo cáo | Slide dự kiến |
|---|---|---|
| TV01 | Mục tiêu, tổ chức, nghiệm thu và kết luận chung | 1–2 |
| TV02 | Mã dòng, ký hiệu, cấu trúc và hàm phụ | 3–4 |
| TV03 | Mở rộng khóa/IV, khởi tạo | 5–6 |
| TV04 | Cập nhật trạng thái, sinh keystream | 7–8 |
| TV05 | API, XOR và streaming | 9–10 |
| TV06 | CLI, validation, ví dụ sử dụng | 11–12 |
| TV07 | Nguồn KAT, Input/Expected/Actual | 13–14 |
| TV08 | File nhị phân, tích hợp, round-trip | 15–16 |
| TV09 | Differential, sanitizer, giới hạn kết luận | 17–18 |
| TV10 | Benchmark, hạn chế, demo và hướng phát triển | 19–20 |

20 slide là đề xuất, điều chỉnh theo thời lượng giảng viên. Mỗi người nộp phần viết, hình/sơ đồ, nguồn và lời thuyết trình; phải trả lời được câu hỏi về module của mình.

## 6. Checklist cá nhân và toàn nhóm

Mỗi thành viên:

- [ ] Issue có GitHub username, deadline, phạm vi và reviewer.
- [ ] File đúng phân công; không còn TODO chặn chức năng bắt buộc.
- [ ] Test có input, oracle/tiêu chí, actual và kết quả thực tế.
- [ ] PR được review; góp ý được xử lý.
- [ ] Đã review phần được phân công và ghi minh chứng.
- [ ] Hoàn thành nội dung báo cáo/slide và nguồn tham khảo.
- [ ] Ghi trung thực hạn chế hoặc việc chưa hoàn thành.

TV01 kiểm tra cuối: đủ đóng góp 10 người; T01–T10 có kết quả; không còn lỗi sai thuật toán/hỏng dữ liệu; clone sạch chạy được; có source báo cáo và slide, PDF, docs/demo.md, results/ và tag nhất quán. Những đường dẫn này là sản phẩm cần tạo ở các mốc sau, chưa được tạo bởi bộ tài liệu này.
