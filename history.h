#ifndef HISTORY_H
#define HISTORY_H

#include <time.h>

#define MAX_URL_LENGTH 256
#define DEFAULT_CAPACITY 50

typedef struct Node Node;

struct Node {
  char url[MAX_URL_LENGTH];
  time_t visitedAt;
  Node *prev;
  Node *next;
};

// head and tail give O(1) access for deleting and adding pages
typedef struct BrowserHistory {
  Node *head;    // oldest page
  Node *tail;    // newest visited page
  Node *current; // current page
  int size;      // number of nodes in the list
  int capacity;  // set limit for the available pages in history
  // TODO stack of deleted nodes
} BrowserHistory;

// Farid
BrowserHistory *historyInit(const char *homepage, int capacity);
int visit(BrowserHistory *bh, const char *url);
const char *historyBack(BrowserHistory *bh, int steps);
const char *historyForward(BrowserHistory *bh, int steps);
const Node *currentPage(const BrowserHistory *bh);
void historyFree(BrowserHistory *bh);

#endif // HISTORY_H
