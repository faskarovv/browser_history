#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "history.h"

// frees every page left in the deleted stack
static void clearDeleted(BrowserHistory *bh) {
  while (bh->deletedTop != NULL) {
    DeletedPage *top = bh->deletedTop;
    bh->deletedTop = top->below;
    free(top->node);
    free(top);
  }
}

BrowserHistory *historyInit(const char *homepage, int capacity) {
  if (homepage == NULL || capacity <= 0) {
    return NULL;
  }

  BrowserHistory *bh = malloc(sizeof(BrowserHistory));
  if (bh == NULL) {
    return NULL;
  }

  Node *node = malloc(sizeof(Node));
  if (node == NULL) {
    free(bh);
    return NULL;
  }
  // filling up the initial node
  strncpy(node->url, homepage, MAX_URL_LENGTH - 1);
  node->url[MAX_URL_LENGTH - 1] = '\0';

  node->visitedAt = time(NULL);
  node->prev = NULL;
  node->next = NULL;

  // linking the browserHistory to the inital node
  bh->head = node;
  bh->tail = node;
  bh->current = node;
  bh->size = 1;
  bh->capacity = capacity;
  bh->deletedTop = NULL;

  return bh;
}

int visit(BrowserHistory *bh, const char *url) {
  if (bh == NULL || url == NULL) {
    return -1;
  }
  // new page
  Node *newNode = malloc(sizeof(Node));
  if (newNode == NULL) {
    return -1;
  }
  // clear all forward history after current
  // if we go back one page and then enter new page the ones come after gets
  // deleted
  // Head<->A<->B<->C and etc
  // if A is the current one we need to delete(free) the other elements
  // so we traverse and free them one by one
  // we hold p->next cause one before it is gonna be invalid after free(b)
  Node *p = bh->current->next; // B
  while (p != NULL) {
    Node *nextNode = p->next;
    free(p);
    p = nextNode;
    bh->size--;
  }

  bh->current->next = NULL;
  bh->tail = bh->current;
  // fill up the new node
  strncpy(newNode->url, url, MAX_URL_LENGTH - 1);
  newNode->url[MAX_URL_LENGTH - 1] = '\0';
  newNode->visitedAt = time(NULL);
  newNode->prev = bh->current;
  newNode->next = NULL;
  // link it with the browser history
  bh->current->next = newNode;
  bh->tail = newNode;
  bh->current = newNode;
  bh->size++;

  // if the capacity is over the given value we delete the node that head
  // pointed to
  if (bh->size > bh->capacity) {
    Node *old = bh->head;
    bh->head = old->next;
    bh->head->prev = NULL;
    free(old);
    bh->size--;
  }

  return 0;
}

const char *historyBack(BrowserHistory *bh, int steps) {
  if (bh == NULL) {
    return NULL;
  }
  // simple traversal back with two conditions
  while (steps > 0 && bh->current->prev != NULL) {
    bh->current = bh->current->prev;
    steps--;
  }

  return bh->current->url;
}

const char *historyForward(BrowserHistory *bh, int steps) {
  if (bh == NULL) {
    return NULL;
  }
  // simple traversal again but forward
  while (steps > 0 && bh->current->next != NULL) {
    bh->current = bh->current->next;
    steps--;
  }

  return bh->current->url;
}

const Node *currentPage(const BrowserHistory *bh) {
  if (bh == NULL) {
    return NULL;
  }

  return bh->current;
}

void historyFree(BrowserHistory *bh) {
  if (bh == NULL) {
    return;
  }
  // we save the p->next cause p is gonna be gone after free(p)
  // and just traverse till the end which is when p == NULL
  Node *p = bh->head;
  while (p != NULL) {
    Node *nextNode = p->next;
    free(p);
    p = nextNode;
  }
  // deleted pages are not in the list anymore so they need their own cleanup
  clearDeleted(bh);
  // free browser history itself
  free(bh);
}

void displayHistory(const BrowserHistory *bh) {
  if (bh == NULL) {
    return;
  }

  for (Node *p = bh->head; p != NULL; p = p->next) {
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M", localtime(&p->visitedAt));
    printf("%s  %s %s\n", timeStr, p->url, p == bh->current ? "[<]" : "");
  }
}

void deleteHistory(BrowserHistory *bh) {
  if (bh == NULL) {
    return;
  }
  // free everything except the current page, browser has to show something
  Node *p = bh->head;
  while (p != NULL) {
    Node *nextNode = p->next;
    if (p != bh->current) {
      free(p);
    }
    p = nextNode;
  }

  bh->current->prev = NULL;
  bh->current->next = NULL;
  bh->head = bh->current;
  bh->tail = bh->current;
  bh->size = 1;
  clearDeleted(bh);
}

int deleteUrl(BrowserHistory *bh, const char *url) {
  // the only page left cant be deleted
  if (bh == NULL || url == NULL || bh->size == 1) {
    return -1;
  }

  Node *p = bh->head;
  int pos = 0;
  while (p != NULL && strcmp(p->url, url) != 0) {
    p = p->next;
    pos++;
  }
  DeletedPage *dp = (p != NULL) ? malloc(sizeof(DeletedPage)) : NULL;
  if (dp == NULL) {
    return -1;
  }

  // if we delete the page we are on, step back (or forward if it was the head)
  if (p == bh->current) {
    bh->current = (p->prev != NULL) ? p->prev : p->next;
  }

  // unlink p, fixing head/tail if it was at one of the ends
  if (p->prev != NULL) p->prev->next = p->next; else bh->head = p->next;
  if (p->next != NULL) p->next->prev = p->prev; else bh->tail = p->prev;
  bh->size--;

  // push onto the stack, the node is kept so restore can reuse it
  dp->node = p;
  dp->position = pos;
  dp->below = bh->deletedTop;
  bh->deletedTop = dp;
  return 0;
}

const char *restoreUrl(BrowserHistory *bh) {
  if (bh == NULL || bh->deletedTop == NULL || bh->size >= bh->capacity) {
    return NULL;
  }

  // pop
  DeletedPage *top = bh->deletedTop;
  Node *node = top->node;
  int pos = top->position;
  bh->deletedTop = top->below;
  free(top);

  // find the node that should come after it, if the list got shorter
  // we just reach NULL and the page goes to the end
  Node *after = bh->head;
  for (int i = 0; i < pos && after != NULL; i++) {
    after = after->next;
  }

  node->next = after;
  node->prev = (after != NULL) ? after->prev : bh->tail;
  if (node->prev != NULL) node->prev->next = node; else bh->head = node;
  if (after != NULL) after->prev = node; else bh->tail = node;
  bh->size++;

  return node->url;
}

// one page per line: "<timestamp> <url>", oldest first
int exportHistory(const BrowserHistory *bh, const char *filename) {
  FILE *f = (bh != NULL && filename != NULL) ? fopen(filename, "w") : NULL;
  if (f == NULL) {
    return -1;
  }

  for (Node *p = bh->head; p != NULL; p = p->next) {
    fprintf(f, "%lld %s\n", (long long)p->visitedAt, p->url);
  }
  fclose(f);
  return 0;
}

// replaces the history with the file, last page in the file becomes current
// visit() already handles the capacity so we just reuse it
int importHistory(BrowserHistory *bh, const char *filename) {
  FILE *f = (bh != NULL && filename != NULL) ? fopen(filename, "r") : NULL;
  if (f == NULL) {
    return -1;
  }

  long long t;
  char url[MAX_URL_LENGTH];
  // 255 = MAX_URL_LENGTH - 1
  if (fscanf(f, "%lld %255s", &t, url) != 2) {
    fclose(f);
    return -1;
  }
  // first page goes into the one node deleteHistory leaves behind
  deleteHistory(bh);
  strcpy(bh->current->url, url);
  bh->current->visitedAt = t;

  while (fscanf(f, "%lld %255s", &t, url) == 2) {
    visit(bh, url);
    bh->current->visitedAt = t;
  }
  fclose(f);
  return 0;
}
