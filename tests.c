#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "history.h"

static void testBackTooFar() {
  BrowserHistory *bh = historyInit("home", 5);
  visit(bh, "A");
  assert(strcmp(historyBack(bh, 100), "home") == 0);
  historyFree(bh);
  printf("testBackTooFar passed\n");
}

int main(void) {
  testBackTooFar();

  return 0;
}
