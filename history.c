#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "history.h"

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
  strncpy(node->url, homepage, MAX_URL_LENGTH - 1);
  node->url[MAX_URL_LENGTH - 1] = '\0';

  node->visitedAt = time(NULL);
  node->prev = NULL;
  node->next = NULL;

  bh->head = node;
  bh->tail = node;
  bh->current = node;
  bh->size = 1;
  bh->capacity = capacity;

  return bh;
}

int visit(BrowserHistory *bh, const char *url) {
  if (bh == NULL || url == NULL) {
    return -1;
  }

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

  strncpy(newNode->url, url, MAX_URL_LENGTH - 1);

  newNode->url[MAX_URL_LENGTH - 1] = '\0';
  newNode->visitedAt = time(NULL);
  newNode->prev = bh->current;
  newNode->next = NULL;

  bh->current->next = newNode;
  bh->tail = newNode;
  bh->current = newNode;
  bh->size++;

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

  Node *p = bh->head;
  while (p != NULL) {
    Node *nextNode = p->next;
    free(p);
    p = nextNode;
  }

  free(bh);
}
