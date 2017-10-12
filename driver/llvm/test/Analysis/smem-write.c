#include <stdint.h>
#include <string.h>

struct hfi_cmd_fake {
  uint32_t size;
};

uint32_t smem[1024];

__attribute__((noinline))
void send_packet(uint8_t *pkt) {
  memcpy((uint8_t *)&smem, pkt, sizeof(uint32_t));
}

__attribute__((noinline))
int main() {
  smem[0] = 1;
  struct hfi_cmd_fake pkt;
  pkt.size = 5;
  send_packet((uint8_t *)&pkt);
  return 0;
}
