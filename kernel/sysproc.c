#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgpte(void)
{
  uint64 va;
  struct proc *p;  

  p = myproc();
  argaddr(0, &va);
  pte_t *pte = pgpte(p->pagetable, va);
  if(pte != 0) {
      return (uint64) *pte;
  }
  return 0;
}
#endif

#ifdef LAB_PGTBL
int
sys_kpgtbl(void)
{
  struct proc *p;  

  p = myproc();
  vmprint(p->pagetable);
  return 0;
}
#endif


uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


uint64
sys_pgaccess(void)
{
    uint64 startva;
    int npages;
    uint64 user_mask_addr;
    uint64 mask = 0;  // Lưu kết quả tạm thời trong kernel
    pte_t *pte;

    // Lấy các tham số từ người dùng
    if (argaddr(0, &startva) < 0 || argint(1, &npages) < 0 || argaddr(2, &user_mask_addr) < 0)
        return -1;

    if (npages < 0 || npages > 32)  // Giới hạn số trang (tùy bạn)
        return -1;

    // Duyệt qua từng trang
    for (int i = 0; i < npages; i++) {
        uint64 va = startva + i * PGSIZE;
        pte = walk(myproc()->pagetable, va, 0);

        if (pte && (*pte & PTE_V)) {  // Kiểm tra PTE hợp lệ
            if (*pte & PTE_A) {  // Kiểm tra bit truy cập
                mask |= (1L << i);  // Đánh dấu trang này trong bitmask
                *pte &= ~PTE_A;  // Xóa bit truy cập để theo dõi lần truy cập tiếp theo
            }
        }
    }

    // Sao chép kết quả từ kernel sang user
    if (copyout(myproc()->pagetable, user_mask_addr, (char *)&mask, sizeof(mask)) < 0)
        return -1;

    return 0;
}
