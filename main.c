#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// reads one line from stdin and removes the newline at the end
// returns 0 if there is nothing to read anymore (ctrl+d / end of file)
static int readLine(const char *prompt, char *buf, int size) {
  printf("%s", prompt);
  fflush(stdout);
  if (fgets(buf, size, stdin) == NULL) {
    return 0;
  }
  buf[strcspn(buf, "\r\n")] = '\0';
  return 1;
}

static int readNumber(const char *prompt) {
  char buf[32];
  if (!readLine(prompt, buf, sizeof(buf))) {
    return 0;
  }
  return atoi(buf);
}

static void printMenu(void) {
  printf("\n----------- Browser -----------\n");
  printf(" 1. Visit a page\n");
  printf(" 2. Back\n");
  printf(" 3. Forward\n");
  printf(" 4. Current page\n");
  printf(" 5. Show history\n");
  printf(" 6. Delete a page from history\n");
  printf(" 7. Restore last deleted page\n");
  printf(" 8. Delete all history\n");
  printf(" 9. Export history to file\n");
  printf("10. Import history from file\n");
  printf(" 0. Exit\n");
  printf("-------------------------------\n");
}

int main() {
  char input[MAX_URL_LENGTH];

  if (!readLine("Homepage: ", input, sizeof(input)) || input[0] == '\0') {
    strcpy(input, "https://www.google.com");
  }

  BrowserHistory *bh = historyInit(input, DEFAULT_CAPACITY);
  if (bh == NULL) {
    printf("Could not start the browser\n");
    return 1;
  }

  int running = 1;
  while (running) {
    printMenu();
    if (!readLine("> ", input, sizeof(input))) {
      break;
    }
    int choice = atoi(input);

    switch (choice) {
    case 1:
      readLine("URL: ", input, sizeof(input));
      if (input[0] == '\0') {
        printf("URL cant be empty\n");
      } else if (visit(bh, input) == 0) {
        printf("Now on: %s\n", currentPage(bh)->url);
      } else {
        printf("Could not visit the page\n");
      }
      break;

    case 2: {
      int steps = readNumber("Steps back: ");
      printf("Now on: %s\n", historyBack(bh, steps));
      break;
    }

    case 3: {
      int steps = readNumber("Steps forward: ");
      printf("Now on: %s\n", historyForward(bh, steps));
      break;
    }

    case 4: {
      const Node *cur = currentPage(bh);
      char timeStr[32];
      strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S",
               localtime(&cur->visitedAt));
      printf("Current page: %s\nVisited at:   %s\n", cur->url, timeStr);
      break;
    }

    case 5:
      displayHistory(bh);
      break;

    case 6:
      readLine("URL to delete: ", input, sizeof(input));
      if (deleteUrl(bh, input) == 0) {
        printf("Deleted. %d page(s) can be restored\n", bh->deletedCount);
      } else {
        printf("Page not found (or it is the only page left)\n");
      }
      break;

    case 7: {
      const char *restored = restoreUrl(bh);
      if (restored != NULL) {
        printf("Restored: %s\n", restored);
      } else if (bh->deletedCount == 0) {
        printf("Nothing to restore\n");
      } else {
        printf("History is full, delete something first\n");
      }
      break;
    }

    case 8:
      deleteHistory(bh);
      printf("History cleared, only the current page is left\n");
      break;

    case 9:
      readLine("File name: ", input, sizeof(input));
      if (exportHistory(bh, input) == 0) {
        printf("Saved %d page(s) to %s\n", bh->size, input);
      } else {
        printf("Could not write to %s\n", input);
      }
      break;

    case 10:
      readLine("File name: ", input, sizeof(input));
      if (importHistory(bh, input) == 0) {
        printf("Loaded %d page(s), now on: %s\n", bh->size,
               currentPage(bh)->url);
      } else {
        printf("Could not load history from %s\n", input);
      }
      break;

    case 0:
      running = 0;
      break;

    default:
      printf("Unknown option\n");
    }
  }

  historyFree(bh);
  return 0;
}
