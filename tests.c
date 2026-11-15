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

  // delete from the middle, current should not move
  assert(deleteUrl(bh, "A") == 0);
  assert(bh->size == 3);
  assert(strcmp(currentPage(bh)->url, "C") == 0);

  // delete the current one, current goes back to B
  assert(deleteUrl(bh, "C") == 0);
  assert(strcmp(currentPage(bh)->url, "B") == 0);
  assert(bh->tail == bh->current);

  assert(deleteUrl(bh, "nothing") == -1);

  // stack so C comes back first, then A in its old spot
  assert(strcmp(restoreUrl(bh), "C") == 0);
  assert(strcmp(restoreUrl(bh), "A") == 0);
  assert(restoreUrl(bh) == NULL);

  const char *expected[] = {"home", "A", "B", "C"};
  int i = 0;
  for (Node *p = bh->head; p != NULL; p = p->next) {
    assert(strcmp(p->url, expected[i]) == 0);
    if (p->next != NULL) {
      assert(p->next->prev == p);
    }
    i++;
  }
  assert(i == 4);
  assert(strcmp(bh->tail->url, "C") == 0);

  historyFree(bh);
  printf("testDeleteAndRestore passed\n");
}

static void testDeleteHistory() {
  BrowserHistory *bh = historyInit("home", 10);
  visit(bh, "A");
  visit(bh, "B");
  visit(bh, "C");
  historyBack(bh, 1);
  deleteUrl(bh, "A");

  deleteHistory(bh);
  assert(bh->size == 1);
  assert(bh->head == bh->tail);
  assert(strcmp(currentPage(bh)->url, "B") == 0);
  assert(restoreUrl(bh) == NULL);

  historyFree(bh);
  printf("testDeleteHistory passed\n");
}

static void testExportImport() {
  BrowserHistory *bh = historyInit("home", 10);
  visit(bh, "https://a.com");
  visit(bh, "https://b.com");
  visit(bh, "https://c.com");
  historyBack(bh, 2);
  assert(exportHistory(bh, "test_history.txt") == 0);

  BrowserHistory *loaded = historyInit("other", 10);
  assert(importHistory(loaded, "test_history.txt") == 0);
  assert(loaded->size == 4);
  assert(strcmp(currentPage(loaded)->url, "https://a.com") == 0);
  assert(strcmp(loaded->head->url, "home") == 0);
  assert(strcmp(loaded->tail->url, "https://c.com") == 0);

  // import into a smaller browser keeps only the newest pages
  BrowserHistory *small = historyInit("x", 2);
  assert(importHistory(small, "test_history.txt") == 0);
  assert(small->size == 2);
  assert(strcmp(small->head->url, "https://b.com") == 0);

  assert(importHistory(bh, "no_such_file.txt") == -1);

  remove("test_history.txt");
  historyFree(bh);
  historyFree(loaded);
  historyFree(small);
  printf("testExportImport passed\n");
}

int main(void) {
  testBackTooFar();
  testDeleteAndRestore();
  testDeleteHistory();
  testExportImport();

  return 0;
}
