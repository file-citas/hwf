#include <stdint.h>
#include <string.h>

uint32_t smem[1024];

__attribute__((noinline))
void write_shm(uint32_t data, uint32_t offset) {
  *(smem + offset) = data;
}

__attribute__((noinline))
int main() {
  smem[0] = 1;
  uint32_t data = 5;
  write_shm(data, 0);
  return 0;
}
