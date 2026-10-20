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
| [Windows build/test](docs/windows-build-test.md) | Cài MSYS2, terminal VS Code, build/test và khắc phục lỗi |
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

Môi trường làm việc chung: **Windows 10/11 x64 + VS Code + MSYS2 UCRT64 + GCC + GNU Make**. Không yêu cầu cài Ubuntu hoặc WSL trên máy thành viên. CI Linux là nơi chạy thêm GCC/Clang và ASan/UBSan.

Xem [hướng dẫn Windows từng bước](docs/windows-build-test.md) để cài công cụ, cấu hình terminal VS Code và xử lý lỗi thường gặp. VS Code không tự kèm compiler; extension C/C++ hỗ trợ soạn thảo/debug, còn GCC thực hiện biên dịch.

Sau khi cài MSYS2, mở **MSYS2 UCRT64** và cập nhật hệ thống:

```bash
pacman -Syu
```

Nếu được yêu cầu đóng terminal, đóng toàn bộ cửa sổ MSYS2, mở lại UCRT64 rồi chạy lại lệnh cập nhật. Sau đó cài công cụ:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb make git diffutils
```

Trong VS Code, mở terminal **MSYS2 UCRT64** đã cấu hình theo hướng dẫn, rồi clone repo vào thư mục làm việc của mình:

```bash
git clone https://github.com/thanhtrung2303/BTL-LTMM-Nhom-4-HC-128.git
cd BTL-LTMM-Nhom-4-HC-128
```

Nếu đã clone, chỉ cần **File → Open Folder** mở repo và tạo terminal UCRT64 tại thư mục gốc.

**Các lệnh dưới đây là giao diện dự kiến, chỉ chạy được sau khi hoàn thành mã nguồn và Makefile. Hiện chưa có chương trình để build/test:**

| Lệnh | Chức năng / môi trường |
|---|---|
| `make` | Build `build/hc128.exe` trên Windows |
| `make test` | Build lại phần thay đổi và chạy bộ test chức năng trên Windows; runner C là `build/test_hc128.exe` |
| `make test-kat` | Chạy test vector và in kết quả đối chiếu |
| `make test-sanitize` | Dành cho CI Linux: build riêng và chạy ASan/UBSan; không bắt thành viên chạy trên UCRT64 |
| `make bench` | Chạy benchmark trên máy Windows, ghi cấu hình đo |
| `make clean` | Xóa sản phẩm build |

Trước khi gửi PR có code, chạy `make` và `make test` trong terminal UCRT64. Nếu sửa lõi/API, chạy thêm KAT và các test streaming/differential liên quan; xem kết quả sanitizer trong tab **Checks** của PR sau khi CI được triển khai. Không ghi PASS khi chưa có test để chạy.

Trên CI Linux, executable không có đuôi `.exe`. Makefile và script phải chọn tên theo nền tảng; không hard-code một tên duy nhất cho cả hai môi trường.

CLI dự kiến có ba chế độ: `keystream`, `encrypt`, `decrypt`. Key và IV mỗi giá trị gồm đúng 32 ký tự hex. Tham số và hành vi chi tiết nằm trong [hợp đồng CLI](docs/module-contracts.md).

## Quy trình đóng góp dành cho người mới

Mỗi người làm trên **nhánh riêng**, sau đó gửi yêu cầu để nhóm kiểm tra và đưa thay đổi vào `main`. Không sửa hoặc đẩy code trực tiếp lên `main`.

### Các khái niệm cần biết

| Từ thường gặp | Hiểu đơn giản |
|---|---|
| Repository / repo | Thư mục dự án và lịch sử thay đổi trên GitHub |
| Clone | Tải repo về máy để làm việc, thường chỉ làm một lần |
| Branch / nhánh | Nơi làm phần việc riêng; `main` là bản chung của nhóm |
| Commit | Lưu một mốc thay đổi vào lịch sử Git trên máy |
| Push | Đưa các commit từ máy lên GitHub |
| Pull | Lấy thay đổi mới từ GitHub về máy |
| Issue | Phiếu ghi nhiệm vụ hoặc lỗi cần xử lý |
| Pull Request / PR | Đề nghị đưa thay đổi từ nhánh riêng vào `main` |
| Review / Approve | Kiểm tra thay đổi / xác nhận đồng ý |
| Merge | Ghép thay đổi đã được kiểm tra vào nhánh chung |

**Lưu ý:** lưu file bằng Ctrl+S chưa phải commit; commit chưa đưa code lên GitHub; push chưa đưa code vào `main`. `git pull` và Pull Request là hai việc khác nhau.

### Bước 1 — Chuẩn bị lần đầu

1. Tạo tài khoản GitHub, gửi **username** cho trưởng nhóm và chấp nhận lời mời tham gia repo.
2. Cài Git qua MSYS2 theo phần chuẩn bị phía trên. Mở terminal **MSYS2 UCRT64** trong VS Code để chạy lệnh.
3. Khai báo tên và email dùng cho commit; thay nội dung ví dụ bằng thông tin của mình:

```bash
git config --global user.name "Ho ten cua ban"
git config --global user.email "email-cua-ban@example.com"
```

Dùng email đã liên kết với GitHub hoặc email noreply do GitHub cấp nếu muốn giữ riêng tư. Thiết lập này ghi danh tính commit, **không phải đăng nhập GitHub**.

Nếu chưa tải repo, chạy lệnh `git clone` ở phần chuẩn bị. Nếu đã clone thì mở lại thư mục đó, không cần clone mỗi lần. Từ bước 3 bên dưới, mọi lệnh Git phải chạy trong thư mục `BTL-LTMM-Nhom-4-HC-128`.

Khi push lần đầu, hoàn tất xác thực GitHub theo công cụ trên máy. Nếu Git yêu cầu mật khẩu qua HTTPS, mật khẩu tài khoản thông thường không dùng được; nhờ trưởng nhóm hỗ trợ thiết lập xác thực. Không gửi token/mật khẩu vào chat, Issue hoặc PR.

### Bước 2 — Nhận nhiệm vụ trên GitHub

Mở tab **Issues** của repo và tìm nhiệm vụ đã được giao. Nếu chưa có, chọn **New issue** để tạo, tránh tạo trùng.

Issue cần ghi: mã TV, việc cần làm, file dự kiến sửa, test cần chạy, người review và hạn hoàn thành. Đọc [kế hoạch nhóm](docs/team-plan.md) để biết phần việc và reviewer của mình.

Ví dụ: `[TV03] Triển khai khởi tạo HC-128`. Ghi nhớ số Issue để liên kết vào PR sau này.

### Bước 3 — Tạo nhánh riêng trước khi sửa file

Ví dụ dưới đây dành cho **TV03 làm phần init**. Thành viên khác thay `feat/TV03-init` bằng nhánh của mình, chẳng hạn `test/TV07-kat` hoặc `docs/TV10-slides`.

```bash
git status
git switch main
git pull --ff-only origin main
git switch -c feat/TV03-init
```

- `git status`: xem nhánh hiện tại và file chưa lưu vào commit. Chỉ chuyển sang nhiệm vụ mới khi không còn thay đổi dở dang; nếu có, hoàn thành/lưu đúng nhánh hoặc hỏi trưởng nhóm trước.
- `git switch main`: chuyển về bản chung trên máy.
- `git pull --ff-only origin main`: lấy bản chung mới nhất.
- `git switch -c ...`: tạo và chuyển sang nhánh làm việc mới.

Nếu một lệnh báo lỗi, dừng để xử lý trước, không tiếp tục sao chép các lệnh phía dưới. Một nhiệm vụ dùng một nhánh; nếu quay lại nhánh đã tạo, dùng `git switch feat/TV03-init` (không thêm `-c`).

### Bước 4 — Làm bài, kiểm tra và commit

Sửa các file được phân công bằng trình soạn thảo, lưu file và chạy test liên quan. Nếu sửa lõi/API, chạy thêm KAT và streaming khi bộ test đã sẵn sàng. Nếu chưa có code/Makefile/test thì ghi rõ chưa chạy, không ghi PASS.

Ví dụ TV03 đã tạo hoặc sửa hai file sau:

```bash
git status
git diff
git add src/hc128_init.c tests/test_init.c
git diff --cached
git commit -m "feat(init): implement HC-128 initialization"
```

Thay hai đường dẫn trên bằng **file thực tế của mình**; không chạy nguyên ví dụ nếu các file chưa tồn tại.

- `git diff`: xem thay đổi ở file đã được Git theo dõi; file mới cần tự mở đọc hoặc xem sau khi add.
- `git add`: chọn file đưa vào commit.
- `git diff --cached`: kiểm tra đúng nội dung sắp commit.
- `git commit -m "..."`: lưu mốc thay đổi kèm mô tả ngắn, rõ việc đã làm.

Chỉ add file liên quan, tránh dùng `git add .` khi chưa kiểm tra. Không commit `build/`, file biên dịch, khóa thật hoặc dữ liệu riêng tư.

### Bước 5 — Đưa nhánh lên GitHub và mở PR

Lần push đầu tiên của nhánh ví dụ:

```bash
git push -u origin feat/TV03-init
```

Sau khi push thành công:

1. Mở repo trên GitHub → **Pull requests → New pull request**.
2. Chọn **base: main** (nhánh nhận) và **compare: feat/TV03-init** (nhánh của mình).
3. Kiểm tra danh sách file và nội dung thay đổi, rồi chọn **Create pull request**.
4. Điền tiêu đề rõ việc đã làm và mô tả theo mẫu dưới đây.
5. Chọn người review trong **Reviewers** theo phân công; nếu không chọn được, báo trưởng nhóm.

```markdown
## Công việc
- TV03: triển khai phần khởi tạo HC-128.
- Closes #12

## File thay đổi
- src/hc128_init.c
- tests/test_init.c

## Kiểm tra
- Lệnh đã chạy:
- Kết quả thực tế:
- Phần chưa chạy hoặc chưa hoàn thành:
```

**Thay `#12` bằng số Issue thật**, sửa nội dung mẫu cho đúng phần mình. `Closes #...` liên kết nhiệm vụ và tự đóng Issue khi PR được merge vào nhánh mặc định.

### Bước 6 — Review và sửa theo góp ý

**Người làm:** đọc góp ý trong PR, sửa file trên chính nhánh đó rồi lưu thêm commit và push:

```bash
git add src/hc128_init.c tests/test_init.c
git commit -m "fix(init): address review feedback"
git push
```

Vẫn thay đường dẫn bằng file thực tế. PR cũ tự cập nhật, **không cần tạo PR mới**. Trả lời từng góp ý, ghi kết quả kiểm tra lại và nhờ reviewer xem lại khi cần.

**Người review:** mở tab **Files changed**, đọc thay đổi, lấy nhánh về chạy kiểm tra liên quan theo phân công. Khi đang ở thư mục repo và không có thay đổi dở dang:

```bash
git fetch origin
git switch --track origin/feat/TV03-init
```

Nếu máy đã có nhánh này, dùng `git switch feat/TV03-init` rồi `git pull --ff-only`. Thay tên nhánh theo PR cần review; không sửa hoặc push vào nhánh của tác giả nếu chưa thống nhất.

Trên PR, chọn **Review changes**:

- **Comment**: gửi nhận xét, chưa xác nhận đồng ý.
- **Request changes**: yêu cầu sửa trước khi merge.
- **Approve**: đồng ý sau khi đã kiểm tra; ghi lệnh và kết quả kiểm tra thực tế.

Tác giả không thể tự approve PR của mình. Viết comment “ok” cũng không được tính là approval.

### Bước 7 — Merge và lấy bản mới về máy

Trưởng nhóm hoặc người được chỉ định thực hiện **Merge pull request** sau khi PR đáp ứng điều kiện review và các kiểm tra bắt buộc. Thành viên không tự merge nếu chưa được giao quyền thực hiện.

Khi PR đã hiện **Merged**, kiểm tra không còn thay đổi dở dang rồi chạy:

```bash
git switch main
git pull --ff-only origin main
```

Từ bản `main` mới này, tạo **nhánh mới** cho nhiệm vụ tiếp theo. Không tiếp tục phát triển trên nhánh của PR đã merge.

### Khi gặp lỗi

| Tình huống | Cách xử lý |
|---|---|
| Push báo không có quyền / lỗi xác thực | Kiểm tra tài khoản, lời mời cộng tác và nhờ trưởng nhóm hỗ trợ |
| Không thấy thay đổi trên GitHub | Kiểm tra đã commit và push thành công chưa; chọn đúng nhánh trên GitHub |
| Không tạo được PR vì không có khác biệt | Kiểm tra base/compare và nhánh đã có commit mới được push chưa |
| Đã có PR cho nhánh | Mở PR hiện có; commit và push tiếp sẽ cập nhật PR đó |
| Không merge được | Đọc thông báo ngay dưới PR: thiếu approval, kiểm tra chưa đạt hoặc conflict |
| Có conflict hoặc pull bị từ chối | Gửi nguyên thông báo cho trưởng nhóm; không tự dùng force-push hay reset --hard |

Quy ước chi tiết: [Conventions](docs/conventions.md). Khi hỏi lỗi, gửi lệnh đã chạy, thông báo lỗi và tên nhánh; che token/mật khẩu nếu có.

## Điều kiện bàn giao

- [ ] Lõi C tự viết; CLI và xử lý file hoạt động theo hợp đồng.
- [ ] KAT khớp từng byte với expected có nguồn độc lập; round-trip không thay thế KAT.
- [ ] Các test bắt buộc đạt, có log thực tế; ASan/UBSan trên CI Linux không báo lỗi trên bộ test.
- [ ] Clone sạch trên Windows UCRT64 có thể build, test và chạy demo theo hướng dẫn; CI Linux cũng đạt các job bắt buộc.
- [ ] Đủ đóng góp và review của 10 thành viên.
- [ ] Mã nguồn, vector, log, báo cáo, slide và demo thống nhất với release `v1.0.0`.

HC-128 trong dự án chỉ mã hóa, không tự xác thực dữ liệu. Không tái sử dụng cùng cặp key–IV cho các bản tin độc lập; key/IV lặp trong test chỉ phục vụ tái lập kết quả.
