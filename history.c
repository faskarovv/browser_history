#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "history.h"

static void freeNodes(Node *p);
static void clearDeleted(BrowserHistory *bh);

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
  bh->deletedCount = 0;

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

// frees every node starting from p till the end of the chain
static void freeNodes(Node *p) {
  while (p != NULL) {
    Node *nextNode = p->next;
    free(p);
    p = nextNode;
  }
}

// empties the deleted pages stack, frees both the stack element and the node
static void clearDeleted(BrowserHistory *bh) {
  while (bh->deletedTop != NULL) {
    DeletedPage *top = bh->deletedTop;
    bh->deletedTop = top->below;
    free(top->node);
    free(top);
  }
  bh->deletedCount = 0;
}

void displayHistory(const BrowserHistory *bh) {
  if (bh == NULL) {
    return;
  }

  printf("History (%d/%d pages):\n", bh->size, bh->capacity);

  int i = 1;
  for (Node *p = bh->head; p != NULL; p = p->next) {
    char timeStr[32];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S",
             localtime(&p->visitedAt));

    printf("%3d. %s  (%s)", i, p->url, timeStr);
    if (p == bh->current) {
      printf("  [<]");
    }
    printf("\n");
    i++;
  }
}

void deleteHistory(BrowserHistory *bh) {
  if (bh == NULL) {
    return;
  }
  // the browser always has to show some page, so we keep the current one
  // and remove everything before and after it
  // A<->B<->[C]<->D  becomes just [C]
  Node *cur = bh->current;

  if (cur->prev != NULL) {
    cur->prev->next = NULL; // cut the left part off so freeNodes stops there
    freeNodes(bh->head);
  }
  freeNodes(cur->next);

  cur->prev = NULL;
  cur->next = NULL;
  bh->head = cur;
  bh->tail = cur;
  bh->size = 1;

  // whole history is wiped so there is nothing to restore either
  clearDeleted(bh);
}

int deleteUrl(BrowserHistory *bh, const char *url) {
  if (bh == NULL || url == NULL) {
    return -1;
  }
  // we dont let the user delete the only page left
  if (bh->size == 1) {
    return -1;
  }

  // find the first node with this url, counting the position as we go
  Node *p = bh->head;
  int pos = 0;
  while (p != NULL && strcmp(p->url, url) != 0) {
    p = p->next;
    pos++;
  }
  if (p == NULL) {
    return -1; // not in history
  }

  DeletedPage *dp = malloc(sizeof(DeletedPage));
  if (dp == NULL) {
    return -1;
  }

  // if we are deleting the page we are on, current moves one back
  // (or one forward if it was the oldest page)
  if (p == bh->current) {
    bh->current = (p->prev != NULL) ? p->prev : p->next;
  }

  // unlink p, head and tail have to be fixed if p was at one of the ends
  if (p->prev != NULL) {
    p->prev->next = p->next;
  } else {
    bh->head = p->next;
  }
  if (p->next != NULL) {
    p->next->prev = p->prev;
  } else {
    bh->tail = p->prev;
  }
  p->prev = NULL;
  p->next = NULL;
  bh->size--;

  // push onto the stack, node is not freed so we can bring it back later
  dp->node = p;
  dp->position = pos;
  dp->below = bh->deletedTop;
  bh->deletedTop = dp;
  bh->deletedCount++;

  return 0;
}

const char *restoreUrl(BrowserHistory *bh) {
  if (bh == NULL || bh->deletedTop == NULL) {
    return NULL;
  }
  // restoring would go over the limit, user has to delete something first
  if (bh->size >= bh->capacity) {
    return NULL;
  }

  // pop the last deleted page
  DeletedPage *top = bh->deletedTop;
  bh->deletedTop = top->below;
  bh->deletedCount--;

  Node *node = top->node;
  int pos = top->position;
  free(top);

  // list could be shorter now than when the page was deleted,
  // in that case it just goes to the end
  if (pos > bh->size) {
    pos = bh->size;
  }

  if (pos == 0) {
    // goes in front of the head
    node->prev = NULL;
    node->next = bh->head;
    bh->head->prev = node;
    bh->head = node;
  } else {
    // walk to the node that will be right before it
    Node *before = bh->head;
    for (int i = 0; i < pos - 1; i++) {
      before = before->next;
    }
    node->prev = before;
    node->next = before->next;
    if (before->next != NULL) {
      before->next->prev = node;
    } else {
      bh->tail = node;
    }
    before->next = node;
  }
  bh->size++;

  return node->url;
}

// file format:
// first line is the index of the current page,
// then one page per line as "<timestamp> <url>" from oldest to newest
int exportHistory(const BrowserHistory *bh, const char *filename) {
  if (bh == NULL || filename == NULL) {
    return -1;
  }

  FILE *f = fopen(filename, "w");
  if (f == NULL) {
    return -1;
  }

  int currentIndex = 0;
  for (Node *p = bh->head; p != bh->current; p = p->next) {
    currentIndex++;
  }
  fprintf(f, "%d\n", currentIndex);

  for (Node *p = bh->head; p != NULL; p = p->next) {
    fprintf(f, "%lld %s\n", (long long)p->visitedAt, p->url);
  }

  fclose(f);
  return 0;
}

int importHistory(BrowserHistory *bh, const char *filename) {
  if (bh == NULL || filename == NULL) {
    return -1;
  }

  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    return -1;
  }

  char line[MAX_URL_LENGTH + 64];
  int currentIndex;
  if (fgets(line, sizeof(line), f) == NULL ||
      sscanf(line, "%d", &currentIndex) != 1) {
    fclose(f);
    return -1;
  }

  // build the new list separately first, so if the file turns out to be
  // empty or broken the old history is still there
  Node *newHead = NULL;
  Node *newTail = NULL;
  int count = 0;

  while (fgets(line, sizeof(line), f) != NULL) {
    line[strcspn(line, "\r\n")] = '\0';

    long long timestamp;
    int urlStart = 0;
    // %n gives us where the url begins after the timestamp and the space
    if (sscanf(line, "%lld %n", &timestamp, &urlStart) != 1 ||
        line[urlStart] == '\0') {
      continue; // skip lines that dont look right
    }

    Node *node = malloc(sizeof(Node));
    if (node == NULL) {
      freeNodes(newHead);
      fclose(f);
      return -1;
    }
    strncpy(node->url, line + urlStart, MAX_URL_LENGTH - 1);
    node->url[MAX_URL_LENGTH - 1] = '\0';
    node->visitedAt = (time_t)timestamp;
    node->prev = newTail;
    node->next = NULL;

    if (newTail != NULL) {
      newTail->next = node;
    } else {
      newHead = node;
    }
    newTail = node;
    count++;

    // same rule as visit(), if the file has more pages than we can hold
    // the oldest ones get dropped
    if (count > bh->capacity) {
      Node *old = newHead;
      newHead = old->next;
      newHead->prev = NULL;
      free(old);
      count--;
      currentIndex--;
    }
  }
  fclose(f);

  if (count == 0) {
    return -1;
  }

  if (currentIndex < 0) {
    currentIndex = 0;
  }
  if (currentIndex >= count) {
    currentIndex = count - 1;
  }

  // now its safe to throw away the old history
  freeNodes(bh->head);
  clearDeleted(bh);

  bh->head = newHead;
  bh->tail = newTail;
  bh->size = count;
  bh->current = newHead;
  for (int i = 0; i < currentIndex; i++) {
    bh->current = bh->current->next;
  }

  return 0;
}
