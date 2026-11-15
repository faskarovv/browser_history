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

static void testDeleteAndRestore() {
  BrowserHistory *bh = historyInit("home", 10);
  visit(bh, "A");
  visit(bh, "B");
  visit(bh, "C");

  assert(deleteUrl(bh, "A") == 0);
  assert(deleteUrl(bh, "C") == 0); // current one, so we move back to B
  assert(strcmp(currentPage(bh)->url, "B") == 0);
  assert(deleteUrl(bh, "nothing") == -1);

  // stack: C comes back first, then A goes back to its old spot
  assert(strcmp(restoreUrl(bh), "C") == 0);
  assert(strcmp(restoreUrl(bh), "A") == 0);
  assert(restoreUrl(bh) == NULL);
  assert(strcmp(bh->head->next->url, "A") == 0);
  assert(strcmp(bh->tail->prev->url, "B") == 0);
  assert(bh->size == 4);

  historyFree(bh);
  printf("testDeleteAndRestore passed\n");
}

static void testExportImport() {
  BrowserHistory *bh = historyInit("home", 10);
  visit(bh, "a.com");
  visit(bh, "b.com");
  assert(exportHistory(bh, "test_history.txt") == 0);

  deleteHistory(bh);
  assert(bh->size == 1);

  assert(importHistory(bh, "test_history.txt") == 0);
  assert(bh->size == 3);
  assert(strcmp(bh->head->url, "home") == 0);
  assert(strcmp(currentPage(bh)->url, "b.com") == 0);
  assert(importHistory(bh, "no_such_file.txt") == -1);

  remove("test_history.txt");
  historyFree(bh);
  printf("testExportImport passed\n");
}

int main(void) {
  testBackTooFar();
  testDeleteAndRestore();
  testExportImport();

  return 0;
}
