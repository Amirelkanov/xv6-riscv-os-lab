#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "check_helpers.c"
#include "kernel/riscv.h"

#define PAGES 4
#define BUF_SIZE (PAGES * PGSIZE)

char stack_buff[BUF_SIZE];

int pgaccess_assert(int ret_code, char *expected_accessed, char *actual_accessed, int num_of_pages) {
  if (ret_code != 0) {
    fprintf(STDERR_D, "Pgaccess error.\n");
    return 1;
  }
  if (memcmp(expected_accessed, actual_accessed, num_of_pages)) {
    fprintf(STDERR_D, "Accesses mismatch. Expected: %s, Got: %s\n", expected_accessed, actual_accessed);
    return 2;
  }
  printf("OK\n");
  return 0;
}

int test_accessed(char *buff, char *expected_accessed) {
  char accessed[PAGES + 1];
  return pgaccess_assert(pgaccess(buff, PAGES, accessed), expected_accessed, accessed, PAGES) == 0;
}

void test_buff(char *buff, int *result) {
  printf("\t[No interaction test]: ");
  *result &= test_accessed(buff, "0000");

  printf("\t[First page interaction test]: ");
  buff[0] = 0;
  *result &= test_accessed(buff, "1000");

  printf("\t[Second & third pages interaction test]: ");
  buff[PGSIZE * 1] = 0;
  buff[PGSIZE * 2] = 0;
  *result &= test_accessed(buff, "0110");

  printf("\t[Forth page interaction test]: ");
  buff[PGSIZE * 3] = 0;
  *result &= test_accessed(buff, "0001");

  printf("\t[No interaction again test]: ");
  *result &= test_accessed(buff, "0000");

  printf("\t[All pages interaction test]: ");
  memset(buff, 0, BUF_SIZE);
  *result &= test_accessed(buff, "1111");
}

void test_heap(int *result) {
  char *heap_buff = malloc(BUF_SIZE);
  printf("<Heap>:\n");
  test_buff(heap_buff, result);
  free(heap_buff);
}

void test_stack(int *result) {
  printf("<Stack>:\n");
  test_buff(stack_buff, result);
}

int main() {
  int result = 1;

  printf("===== Page access tests started =====\n");

  test_stack(&result);
  printf("==========\n");
  test_heap(&result);

  printf("===== %s =====\n", (result) ? "All tests have passed successfully!" : "Some tests have failed");

  return 0;
}