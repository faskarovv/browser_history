#include "history.h"
#include <stdio.h>

int main() {
  BrowserHistory *bh = historyInit("https://www.google.com", DEFAULT_CAPACITY);
  if (bh == NULL) {
    return 1;
  }

  char text[MAX_URL_LENGTH]; // url or file name
  int choice = -1, steps;

  while (choice != 0) {
    printf("\n1 visit  2 back  3 forward  4 current  5 history\n"
           "6 delete  7 restore  8 delete all  9 export  10 import  0 exit\n> ");
    if (scanf("%d", &choice) != 1) {
      break;
    }

    switch (choice) {
    case 1:
      printf("URL: ");
      scanf("%255s", text);
      visit(bh, text);
      break;
    case 2:
      printf("Steps: ");
      scanf("%d", &steps);
      historyBack(bh, steps);
      break;
    case 3:
      printf("Steps: ");
      scanf("%d", &steps);
      historyForward(bh, steps);
      break;
    case 4:
      printf("%s, visited %s", currentPage(bh)->url,
             ctime(&currentPage(bh)->visitedAt));
      break;
    case 5:
      displayHistory(bh);
      break;
    case 6:
      printf("URL: ");
      scanf("%255s", text);
      if (deleteUrl(bh, text) != 0) {
        printf("Not found or it is the only page\n");
      }
      break;
    case 7:
      if (restoreUrl(bh) == NULL) {
        printf("Nothing to restore or history is full\n");
      }
      break;
    case 8:
      deleteHistory(bh);
      break;
    case 9:
    case 10:
      printf("File: ");
      scanf("%255s", text);
      if ((choice == 9 ? exportHistory(bh, text) : importHistory(bh, text)) != 0) {
        printf("Could not open %s\n", text);
      }
      break;
    }
    // show where we are after every command
    if (choice >= 1 && choice <= 10 && choice != 4 && choice != 5) {
      printf("Now on: %s\n", currentPage(bh)->url);
    }
  }

  historyFree(bh);
  return 0;
}
