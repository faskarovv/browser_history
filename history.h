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

// element of the deleted pages stack
// we also keep the position the page had so restore can put it back there
typedef struct DeletedPage DeletedPage;

struct DeletedPage {
  Node *node;
  int position; // index in the list when it got deleted (head is 0)
  DeletedPage *below;
};

// head and tail give O(1) access for deleting and adding pages
typedef struct BrowserHistory {
  Node *head;    // oldest page
  Node *tail;    // newest visited page
  Node *current; // current page
  int size;      // number of nodes in the list
  int capacity;  // set limit for the available pages in history
  DeletedPage *deletedTop; // top of the stack of deleted pages
  int deletedCount;
} BrowserHistory;

// Farid
BrowserHistory *historyInit(const char *homepage, int capacity);
int visit(BrowserHistory *bh, const char *url);
const char *historyBack(BrowserHistory *bh, int steps);
const char *historyForward(BrowserHistory *bh, int steps);
const Node *currentPage(const BrowserHistory *bh);
void historyFree(BrowserHistory *bh);

// Toghrul
void displayHistory(const BrowserHistory *bh);
void deleteHistory(BrowserHistory *bh);
int deleteUrl(BrowserHistory *bh, const char *url);
const char *restoreUrl(BrowserHistory *bh);
int exportHistory(const BrowserHistory *bh, const char *filename);
int importHistory(BrowserHistory *bh, const char *filename);

#endif // HISTORY_H
