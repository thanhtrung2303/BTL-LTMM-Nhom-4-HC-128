# Ghi chú đặc tả HC-128

> Chủ trì: TV02; reviewer: TV03. Cơ sở phạm vi là [README](../README.md). Ghi chú này định hướng triển khai và review, không thay thế đặc tả gốc hay chứng nhận độ đúng của code.

## 1. Mục tiêu và tham số

Tự triển khai HC-128 bằng C; trình bày thuật toán và kiểm định Input/output theo yêu cầu giảng viên. Không dùng thư viện HC-128 để thay lõi tự viết; triển khai độc lập chỉ dùng làm oracle.

| Thành phần | Quy ước |
|---|---|
| Key | 128 bit = 16 byte |
| IV | 128 bit = 16 byte |
| Word | 32 bit unsigned |
| P, Q | Mỗi bảng 512 word |
| Counter | Modulo 1024 |
| Đầu ra mỗi bước | Một word keystream 32 bit |
| Byte của word | Little-endian tường minh |
| Mã hóa / giải mã | XOR dữ liệu với cùng dòng khóa từ context mới |

Phép cộng word phải có kết quả modulo 2^32. Không áp dụng modulo 1024 cho chỉ số truy cập bảng 512 phần tử. Không đồng nhất số byte đã trả cho caller với số word đã sinh nội bộ vì có byte dư.

## 2. Các giai đoạn phải hiện thực

1. Nhận đúng 16 byte key và 16 byte IV; xác định thứ tự byte trước khi mở rộng.
2. Mở rộng khóa/IV theo đặc tả tác giả, nạp P và Q.
3. Thực hiện giai đoạn khởi tạo 1024 bước với cơ chế phản hồi vào bảng đúng đặc tả.
4. Đặt trạng thái bắt đầu xuất dòng khóa, reset counter và bộ đệm byte.
5. Với mỗi bước sinh, chọn nhánh P hoặc Q theo counter; cập nhật đúng bảng và sinh một word.
6. Tuần tự xuất byte của word theo little-endian; giữ phần chưa dùng cho lần gọi sau.
7. XOR với input nếu caller yêu cầu mã hóa/giải mã.

**Điểm dễ sai:** giai đoạn khởi tạo có ghi phản hồi vào bảng. Gọi hàm sinh bình thường 1024 lần rồi bỏ output không được mặc nhiên coi là tương đương. TV03/04 phải đối chiếu cách cập nhật với nguồn và snapshot độc lập.

## 3. Hồ sơ công thức cần chốt ở M1

README yêu cầu TV02 trình bày đầy đủ f1, f2, g1, g2, h1, h2. Bộ ghi chú này không tự bổ sung công thức hoặc expected chưa kiểm chứng. Trước khi triển khai, TV02 phải bổ sung bảng công thức được trích đúng nguồn và được TV03 review:

| Mục cần xác minh | Nội dung phải ghi | Người áp dụng |
|---|---|---|
| f1, f2 | Phép xoay/dịch, số đếm, kiểu word, phương trình | TV02/03 |
| Mở rộng khóa | Thứ tự word key/IV, bước lặp, miền chỉ số, hằng số | TV03 |
| Nạp P/Q | Miền word đầu vào tương ứng từng bảng | TV03 |
| g1, g2 | Tham số, chiều rotate, cộng/XOR, thứ tự đánh giá | TV04 |
| h1, h2 | Byte được chọn, chỉ số bảng, phép kết hợp | TV04 |
| Warm-up | Thứ tự cập nhật/ghi phản hồi, số bước và counter cuối | TV03/04 |
| Sinh word | Nhánh P/Q, chỉ số modulo 512, vị trí tăng counter | TV04 |
| Byte output | Thứ tự byte và cách ánh xạ vector nguồn | TV05/07 |

Điều kiện đóng M1: ghi phiên bản tài liệu, mục/trang của mỗi công thức; reviewer kiểm tra độc lập; không còn ô chưa xác minh ảnh hưởng code. Đây là đầu việc có chủ trì, không phải kết quả đã hoàn tất.

## 4. Bất biến cần kiểm tra

- P/Q có đúng 512 phần tử mỗi bảng; mọi chỉ số nằm trong 0..511.
- Counter luôn nằm trong 0..1023 sau bước hoàn tất.
- Byte dư trong context thuộc word gần nhất đã sinh; không bị bỏ khi đổi từ keystream sang XOR hoặc ngược lại.
- Khởi tạo lại cùng key/IV tái lập cùng dòng byte, kể cả context cũ đang giữ byte dư.
- Các context độc lập; gọi xen kẽ không ảnh hưởng nhau.
- len=0 không tiêu thụ dòng khóa.
- Tách input thành các chunk bất kỳ cho cùng output như xử lý một lần.
- Giải mã bắt đầu với context mới cùng key/IV, không dùng tiếp context mã hóa.

## 5. Ánh xạ đặc tả sang module và test

| Nội dung | Module dự kiến | Test |
|---|---|---|
| Phép toán 32 bit/endian | src/hc128_internal.h | T02 |
| Mở rộng, nạp bảng, warm-up | src/hc128_init.c | T03, T07 |
| Bước sinh/counter | src/hc128_core.c | T04, T07, T09 |
| Byte dư/XOR | src/hc128_stream.c | T05, T07, T09 |
| Giao diện/đọc ghi | app/ | T06, T08 |

Xem [hợp đồng module](module-contracts.md) và [kế hoạch kiểm thử](test-plan.md) để biết tiền điều kiện, ca biên và minh chứng.

## 6. Nguồn và quy tắc kiểm chứng

Các liên kết dưới đây kế thừa nguồn tham khảo trong README, chưa được xác nhận truy cập lại trong lần lập bộ docs này:

1. Hongjun Wu, *The Stream Cipher HC-128*, LNCS 4986, 2008, trang 39–47: [PDF eSTREAM](https://www.ecrypt.eu.org/stream/p3ciphers/hc/hc128_p3.pdf).
2. [The eSTREAM Portfolio, bản lưu tại CORDIS](https://cordis.europa.eu/docs/projects/cnect/6/216676/080/deliverables/001-DSYM10v1.pdf).
3. [rand_hc: mã nguồn và test](https://docs.rs/rand_hc/latest/src/rand_hc/hc128.rs.html): chỉ là đầu mối tìm implementation độc lập; phải cố định phiên bản trước sử dụng, không dùng latest làm provenance cuối.
4. [Dự án eSTREAM](https://www.ecrypt.eu.org/stream/).

Nếu nguồn gốc không truy cập được, ghi nguồn bản sao, phiên bản, checksum, ngày lấy và cách kiểm tra chéo; không tự điền công thức/expected từ trí nhớ rồi ghi là đã kiểm chứng. tests/vectors/README.md phải quản lý nguồn từng case riêng.

## 7. Giới hạn

HC-128 ở phạm vi bài tập chỉ mã hóa, không tự xác thực. Sai key/IV hoặc ciphertext bị sửa có thể vẫn cho output và exit code thành công. Không thêm yêu cầu “phát hiện sai khóa” vào test. Round-trip không đủ chứng minh phù hợp đặc tả. KAT/differential chỉ xác nhận các trường hợp đã chạy, không chứng minh tuyệt đối an toàn mật mã.
