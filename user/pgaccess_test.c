#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "check_helpers.c"
#include "kernel/riscv.h"

#define BUF_SIZE 1024

char stack_buff[BUF_SIZE];

void test_accessed(char *buff, int expected_accessed) {
  myassert(pgaccess(buff, BUF_SIZE) == expected_accessed, "Accesses mismatch.\n");
  printf("OK\n");
}

void test_buff(char *buff) {
  pgaccess(buff, BUF_SIZE); // Сбросим биты доступа на всякий

  printf("\t[No interaction test]: ");
  test_accessed(buff, 0);

  printf("\t[First element interaction test]: ");
  buff[0] = 0;
  test_accessed(buff, 1);

  printf("\t[Last element interaction test]: ");
  buff[BUF_SIZE - 1] = 0;
  test_accessed(buff, 1);

  printf("\t[No interaction again test]: ");
  test_accessed(buff, 0);
}

void test_heap(void) {
  char *heap_buff = malloc(BUF_SIZE);
  printf("<Heap>:\n");
  test_buff(heap_buff);
  free(heap_buff);
}

void test_stack(void) {
  printf("<Stack>:\n");
  test_buff(stack_buff);
}

int main() {
  printf("===== Page access tests started =====\n");

  test_stack();
  printf("==========\n");
  test_heap();

  printf("===== All tests have passed successfully! =====\n");

  return 0;
}