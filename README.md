# HC-128 — Triển khai mã dòng bằng C

Bài tập lớn môn **Lý thuyết mật mã**, **nhóm 4**, giảng viên **TS. Tạ Thị Kim Huệ**.

Dự án tự triển khai thuật toán HC-128 trong eSTREAM bằng C, xây dựng chương trình mã hóa/giải mã và kiểm định bằng test vector có nguồn độc lập.

> **Trạng thái:** repository hiện có tài liệu và khung thư mục. Mã nguồn C, Makefile, CI và bộ test chưa được triển khai; chưa có kết quả kiểm thử.

## Mục tiêu và sản phẩm

- Trình bày nguyên lý, quá trình khởi tạo và sinh dòng khóa của HC-128.
- Tự viết lõi bằng C; hỗ trợ mã hóa/giải mã dữ liệu và file qua CLI.
- Chạy test vector, ghi rõ **Input, Expected Output, Actual Output và PASS/FAIL**.
- Bàn giao mã nguồn, dữ liệu test, log kiểm thử, báo cáo, slide và demo.

Lõi thuật toán tách khỏi CLI và xử lý file. Thư viện bên ngoài chỉ dùng làm đối chứng kiểm thử, không thay lõi tự viết. GUI và giao thức mạng nằm ngoài phạm vi.

## Thành viên

| Mã | Họ và tên | MSSV | Phụ trách |
|---|---|---|---|
| TV01 | Nguyễn Thành Trung | 20233680 | Trưởng nhóm, tích hợp, build và CI |
| TV02 | Tăng Tiến Dũng | 20233350 | Đặc tả và phép toán 32 bit |
| TV03 | Phạm Quốc Cường | 20233295 | Mở rộng khóa và khởi tạo |
| TV04 | Đồng Xuân Quang | 20233603 | Cập nhật trạng thái, sinh keystream |
| TV05 | Vũ Khắc Hùng | 20233428 | API byte, mã hóa và giải mã |
| TV06 | Đặng Quốc Anh | 20233240 | CLI và kiểm tra đầu vào |
| TV07 | Nguyễn Anh Tú | 20233691 | Test vector và bộ chạy KAT |
| TV08 | Nguyễn Tuấn Việt | 20233713 | Xử lý file, kiểm thử tích hợp |
| TV09 | Trần Thái Sơn | 20233625 | Đối chiếu độc lập, sanitizer |
| TV10 | Trần Văn Vinh | 20223829 | Benchmark, tổng hợp tài liệu và demo |

Nhiệm vụ, reviewer, mốc nghiệm thu và phân chia báo cáo/slide: [Kế hoạch nhóm](docs/team-plan.md).

## Tài liệu hướng dẫn

| Tài liệu | Nội dung |
|---|---|
| [Conventions](docs/conventions.md) | Quy ước C, commit, PR và quản lý dữ liệu |
| [Team plan](docs/team-plan.md) | Phân công, review chéo, tiến độ và bàn giao |
| [Spec notes](docs/spec-notes.md) | Ghi chú thuật toán, điểm cần xác minh và tài liệu tham khảo |
| [Module contracts](docs/module-contracts.md) | API, trạng thái, giao tiếp module và CLI |
| [Test plan](docs/test-plan.md) | Ma trận T01–T10, nguồn expected, log và nghiệm thu |

Thành viên đọc kế hoạch nhóm và quy ước trước khi bắt đầu; người viết code cần đọc thêm hợp đồng module và các test liên quan.

## Cấu trúc dự án

Các thư mục hiện được giữ bằng `.gitkeep` khi chưa có nội dung triển khai.

| Đường dẫn | Mục đích |
|---|---|
| `include/` | Header và API công khai |
| `src/` | Khởi tạo, lõi HC-128 và xử lý byte |
| `app/` | CLI, chuyển đổi hex, đọc/ghi file |
| `tests/` | Unit test và kiểm thử tích hợp |
| `tests/vectors/` | Test vector, nguồn và manifest |
| `tests/fixtures/` | Dữ liệu mẫu công khai |
| `tools/reference/` | Adapter và triển khai đối chứng |
| `bench/` | Đo hiệu năng |
| `docs/` | Tài liệu thiết kế và hướng dẫn nhóm |
| `docs/report/`, `docs/slides/` | Báo cáo và slide |
| `results/` | Log, số liệu và môi trường bản bàn giao |
| `.github/workflows/` | Workflow CI cần triển khai |
| `build/` | Sản phẩm sinh khi build; không commit |

## Chuẩn bị và chạy dự án

Môi trường chuẩn: **Ubuntu hoặc Ubuntu trên WSL**, C11, GCC/Clang, GNU Make và Git.

```bash
sudo apt update
sudo apt install build-essential clang git

git clone https://github.com/thanhtrung2303/BTL-LTMM-Nhom-4-HC-128.git
cd BTL-LTMM-Nhom-4-HC-128
```

**Các lệnh dưới đây là giao diện dự kiến, chỉ chạy được sau khi hoàn thành mã nguồn và Makefile:**

| Lệnh | Chức năng |
|---|---|
| `make` | Build chương trình `build/hc128` |
| `make test` | Chạy bộ kiểm thử chức năng |
| `make test-kat` | Chạy test vector và in kết quả đối chiếu |
| `make test-sanitize` | Kiểm tra bằng ASan/UBSan |
| `make bench` | Chạy benchmark |
| `make clean` | Xóa sản phẩm build |

CLI dự kiến có ba chế độ: `keystream`, `encrypt`, `decrypt`. Key và IV mỗi giá trị gồm đúng 32 ký tự hex. Tham số và hành vi chi tiết nằm trong [hợp đồng CLI](docs/module-contracts.md).

## Quy trình đóng góp

1. Tạo Issue ghi nhiệm vụ, người phụ trách, reviewer và tiêu chí nghiệm thu.
2. Tạo nhánh riêng từ `main`, ví dụ `feat/TV03-init`.
3. Thực hiện công việc và chạy test liên quan; sửa lõi/API phải chạy thêm KAT và streaming.
4. Mở PR liên kết Issue, ghi thay đổi và kết quả kiểm tra thực tế.
5. Review chéo, xử lý góp ý và đáp ứng các điều kiện bảo vệ nhánh trước khi merge.

Không tự approve PR của mình; không force-push nhánh chung. Không commit sản phẩm build, khóa thật hoặc dữ liệu riêng tư. Chi tiết: [Conventions](docs/conventions.md).

## Điều kiện bàn giao

- [ ] Lõi C tự viết; CLI và xử lý file hoạt động theo hợp đồng.
- [ ] KAT khớp từng byte với expected có nguồn độc lập; round-trip không thay thế KAT.
- [ ] Các test bắt buộc đạt, có log thực tế; ASan/UBSan không báo lỗi trên bộ test.
- [ ] Clone sạch có thể build, test và chạy demo theo hướng dẫn.
- [ ] Đủ đóng góp và review của 10 thành viên.
- [ ] Mã nguồn, vector, log, báo cáo, slide và demo thống nhất với release `v1.0.0`.

HC-128 trong dự án chỉ mã hóa, không tự xác thực dữ liệu. Không tái sử dụng cùng cặp key–IV cho các bản tin độc lập; key/IV lặp trong test chỉ phục vụ tái lập kết quả.
