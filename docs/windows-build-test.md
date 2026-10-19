# Build và chạy test trên Windows bằng VS Code

Môi trường chung của nhóm: **Windows 10/11 x64 + VS Code + MSYS2 UCRT64 + GCC + GNU Make**. Các bước dưới đây không yêu cầu Ubuntu/WSL. VS Code là nơi viết code và mở terminal; GCC biên dịch chương trình C, Make điều phối build/test.

> Repo hiện có tài liệu và khung thư mục, chưa có Makefile, mã C, bộ test hay CI. Có thể cài và kiểm tra công cụ ngay. Các lệnh `make ...` và chạy `.exe` là giao diện TV01 và các thành viên cần hiện thực; chúng chưa phải kết quả chạy thành công.

## 1. Cài công cụ một lần

1. Cài [VS Code](https://code.visualstudio.com/) trên Windows; cài extension **C/C++** của Microsoft trong mục Extensions. Extension không tự cài compiler.
2. Cài [MSYS2](https://www.msys2.org/) bản x64. Hướng dẫn dùng đường dẫn mặc định `C:\msys64`; nếu cài nơi khác, sửa đường dẫn cấu hình tương ứng.
3. Từ Start Menu, mở **MSYS2 UCRT64** và cập nhật:

```bash
pacman -Syu
```

Nếu trình cập nhật yêu cầu đóng terminal, đóng các cửa sổ MSYS2, mở lại **MSYS2 UCRT64** rồi chạy lại `pacman -Syu` cho tới khi cập nhật hoàn tất.

4. Cài GCC native Windows, debugger, GNU Make, Git và công cụ so file:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb make git diffutils
```

Ở đây dùng gói `make` của MSYS2 để chạy recipe Bash. Lệnh chuẩn là `make`, không đổi tùy ý sang `mingw32-make`. GCC phải thuộc UCRT64; không trộn với compiler MSYS/MINGW64 hoặc GCC của bộ công cụ khác.

## 2. Tạo terminal UCRT64 trong VS Code

Nhấn **Ctrl+Shift+P → Preferences: Open User Settings (JSON)**, thêm các khóa sau vào đối tượng JSON hiện có. Không xóa cấu hình cá nhân khác; nếu đã có `terminal.integrated.profiles.windows`, thêm profile vào đối tượng đó thay vì lặp lại khóa.

```json
{
    "terminal.integrated.profiles.windows": {
        "MSYS2 UCRT64": {
            "path": "C:\\msys64\\usr\\bin\\bash.exe",
            "args": ["--login", "-i"],
            "env": {
                "MSYSTEM": "UCRT64",
                "CHERE_INVOKING": "1"
            }
        }
    },
    "terminal.integrated.defaultProfile.windows": "MSYS2 UCRT64"
}
```

Đóng terminal cũ bằng biểu tượng thùng rác, chọn **Terminal → New Terminal**. Kiểm tra:

```bash
echo "$MSYSTEM"
command -v gcc
command -v make
gcc --version
make --version
git --version
```

Kết quả đích: `UCRT64`, `/ucrt64/bin/gcc` và `/usr/bin/make` (có thể có đuôi `.exe`). Nếu sai, chọn lại profile hoặc sửa cài đặt trước khi build. Không chỉ đổi tên terminal rồi cho rằng đã chuyển môi trường.

Các lệnh Bash, `pacman`, `make` trong tài liệu đều chạy ở terminal UCRT64 này, không dán vào PowerShell/CMD. Không cần thêm toàn bộ `C:\msys64\usr\bin` vào PATH Windows toàn hệ thống.

## 3. Mở repository

Nếu đã clone: chọn **File → Open Folder**, mở thư mục `BTL-LTMM-Nhom-4-HC-128` rồi mở terminal mới. Không clone lại mỗi lần làm việc.

Nếu chưa clone, ví dụ lưu tại `C:\projects` thì trong UCRT64 chạy:

```bash
mkdir -p /c/projects
cd /c/projects
git clone https://github.com/thanhtrung2303/BTL-LTMM-Nhom-4-HC-128.git
cd BTL-LTMM-Nhom-4-HC-128
```

Sau đó dùng **File → Open Folder** mở thư mục đó trong VS Code. `/c/projects` là cách Bash MSYS2 biểu diễn `C:\projects`; không dùng nguyên đường dẫn dấu gạch chéo ngược Windows trong Bash. Nếu có khoảng trắng, đặt đường dẫn trong dấu nháy.

Kiểm tra đang ở gốc repo:

```bash
pwd
git status
ls
```

## 4. Build/test khi code và Makefile đã sẵn sàng

```bash
make
make test
```

| Lệnh | Sản phẩm / hành vi đích |
|---|---|
| `make` | Biên dịch lõi + ứng dụng thành `build/hc128.exe` |
| `make test` | Build test và ứng dụng cần thiết, chạy unit/KAT/streaming/differential và test CLI/file |
| `make test-kat` | Chạy riêng KAT, in Input/Expected/Actual và PASS/FAIL |
| `make bench` | Build/chạy benchmark; chỉ đo sau khi xác minh output |
| `make clean` | Xóa sản phẩm build, giữ vector và log bàn giao |

Runner C dự kiến là `build/test_hc128.exe`. `make test` còn phải gọi các script test CLI/file nếu dùng script, nên chạy riêng runner C chưa thay thế toàn bộ target này.

Sau khi build, ví dụ chạy CLI dự kiến:

```bash
./build/hc128.exe --help
./build/hc128.exe keystream --key 00000000000000000000000000000000 --iv 00000000000000000000000000000000 --bytes 64
```

Không dùng nút **Run Code** để biên dịch riêng file `.c` đang mở cho dự án nhiều module. Build phải liên kết đủ file qua Makefile; app và test runner có hai hàm `main()` thuộc hai executable khác nhau.

Khi cần build lại sạch:

```bash
make clean
make
make test
```

Lưu minh chứng KAT và kiểm tra ngay mã thoát:

```bash
mkdir -p results
make test-kat > results/kat.log 2>&1
test_status=$?
cat results/kat.log
printf 'Exit code: %s\n' "$test_status"
```

Mã `0` chỉ có ý nghĩa khi runner thực sự chạy đủ các case thuộc suite được yêu cầu. Runner phải trả khác `0` khi thiếu vector, không chạy được case hoặc có FAIL. Không dùng đoạn in log trên để thay thế job CI: CI gọi trực tiếp `make test` và giữ mã lỗi của lệnh đó.

## 5. Sanitizer và GitHub Actions

**Máy thành viên:** build/test chức năng bằng UCRT64/GCC. Không bắt buộc cài Clang hoặc chạy ASan/UBSan trên Windows.

**CI cần triển khai:** TV01 cấu hình job Windows với MSYS2 UCRT64 chạy cùng `make`/`make test`; job Linux chạy GCC/Clang và `make test-sanitize`. TV09 kiểm tra sanitizer và lưu log của đúng commit. Đây là yêu cầu đích, hiện chưa có workflow thực thi.

Khi CI đã hoạt động, mở PR → tab **Checks** để xem kết quả. Test Windows đạt không thay thế sanitizer Linux. Lỗi ở job bắt buộc phải được xử lý trước merge/release; không ghi SKIP thành PASS. Linux tạo executable không có `.exe`; Makefile/script chịu trách nhiệm chọn tên theo nền tảng.

## 6. Lỗi thường gặp

| Lỗi | Cách kiểm tra / sửa |
|---|---|
| `gcc: command not found` | Cài gói GCC UCRT64 và kiểm tra `MSYSTEM`, `command -v gcc` |
| `make: command not found` | Cài `pacman -S --needed make`, mở đúng terminal UCRT64 |
| Không có Makefile / không có target | Kiểm tra `pwd`, `ls`; nếu repo chưa triển khai thì đây là trạng thái chưa có build, không phải lỗi cài VS Code |
| `missing separator` trong Makefile | Recipe cần tab thật; không thay bằng 4 dấu cách |
| `multiple definition of main` | Tách executable ứng dụng và test; không link cả `app/main.c` và `tests/test_main.c` cùng nhau |
| `undefined reference` | Kiểm tra hàm đã triển khai và file `.c` tương ứng đã nằm trong danh sách link |
| Script báo `$'\r'` hoặc `bad interpreter` | Chuyển line ending script sang LF trong VS Code; không chuyển fixture nhị phân |
| `.exe` không tồn tại | Đọc lỗi build đầu tiên, không chạy tiếp khi build thất bại |
| Thiếu DLL khi chạy từ Explorer | Chạy trong terminal UCRT64 đã cấu hình, nơi có đường dẫn runtime phù hợp |
| `make test-sanitize` không hỗ trợ Windows | Xem job sanitizer Linux; không bỏ cờ rồi ghi là đã kiểm tra sanitizer |

Khi báo lỗi, gửi lệnh, toàn bộ thông báo lỗi, nhánh/commit, `echo "$MSYSTEM"`, `command -v gcc` và `gcc --version`. Không gửi token/mật khẩu.

## 7. Checklist trước PR

- [ ] Đúng terminal UCRT64 và GCC từ `/ucrt64/bin`.
- [ ] `make` và `make test` chạy đạt nếu phần code/test đã sẵn sàng; nếu chưa, ghi rõ chưa chạy và lý do.
- [ ] Sửa lõi/API đã chạy KAT và test streaming/differential liên quan.
- [ ] Script text lưu UTF-8/LF, Makefile giữ tab recipe, không commit `build/` hoặc `.exe`.
- [ ] PR ghi môi trường và kết quả thực tế; reviewer kiểm tra job Windows/Linux khi CI đã triển khai.

Nguồn cấu hình: [MSYS2](https://www.msys2.org/), [môi trường UCRT64](https://www.msys2.org/docs/environments/), [terminal profiles của VS Code](https://code.visualstudio.com/docs/terminal/profiles), [GCC/MinGW trong VS Code](https://code.visualstudio.com/docs/cpp/config-mingw), [MSYS2 trong CI](https://www.msys2.org/docs/ci/).
