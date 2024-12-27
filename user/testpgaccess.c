#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    // 1. Tạo một vùng bộ nhớ mới với `sbrk`
    char *addr = sbrk(4096 * 10); // Tạo 10 trang bộ nhớ (40KB)

    // 2. Truy cập một vài trang trong vùng bộ nhớ
    addr[0] = 'A';         // Truy cập trang đầu tiên
    addr[8192] = 'B';      // Truy cập trang thứ ba
    addr[40960 - 1] = 'C'; // Truy cập trang cuối cùng (trang thứ 10)

    // 3. Tạo một bitmask để nhận kết quả
    uint64 mask = 0;

    // 4. Gọi `pgaccess` để kiểm tra các trang đã được truy cập
    if (pgaccess(addr, 10, &mask) < 0) {
        printf("pgaccess failed\n");
        exit(1);
    }

    // 5. In kết quả
    printf("Accessed pages: 0x%lx\n", mask); // In bitmask các trang được truy cập

    // Kiểm tra nếu kết quả đúng
    if (mask == 0x205) { // Trang 0, 2, và 9 đã được truy cập
        printf("test passed!\n");
    } else {
        printf("test failed: mask=0x%lx\n", mask);
    }

    exit(0);
}
