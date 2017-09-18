#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct {
  char* said;
  int len;
} quote;

quote** quotes = NULL;
int n_quotes = 0;

void die(const char *msg) {
    perror(msg);
    exit(1);
}

char* read_quotes_file() {
  int size;
  char* inp;
  FILE* f = fopen("quotes.txt", "r");

  if (!f) die("error reading quotes file");

  fseek(f, 0, SEEK_END);
  size = ftell(f);
  fseek(f, 0, SEEK_SET);

  inp = malloc(size+1);
  fread(inp, size, 1, f);
  fclose(f);

  inp[size] = '\0';

  return inp;
}

void make_quotes(char* contents) {
  char* line;
  quote* q;

  while ((line = strsep(&contents, "\n"))) {
    if (!strcmp(line, "")) continue;
    q = malloc(sizeof(quote));
    q->said = line;
    q->len = strlen(line);
    quotes = realloc(quotes, (++n_quotes)*sizeof(quote*));
    quotes[n_quotes-1] = q;
  }
}

void initialize_quotd() {
  char* contents = read_quotes_file();

  make_quotes(contents);
}

void cleanup_quotd() {
  int i;

  for (i = 0; i < n_quotes; i++) free(quotes[i]);
  free(quotes);
}

int get_random_from_day() {
  time_t t;
  struct tm* tmp;
  t = time(NULL);
  tmp = localtime(&t);

  srand(tmp->tm_yday); // hack to provide determinism per day.

  return rand();
}

char* get_quotd(int* len) {
  quote* q;
  char* res;
  int n;
  n = get_random_from_day() % n_quotes;
  q = quotes[n];
  res = malloc(q->len+2);

  snprintf(res, q->len+2, "%s\n", q->said);
  *len = q->len+2;

  return res;
}

int main(int argc, char *argv[]) {
  int sockfd;
  int newsockfd;
  int port = 17;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  initialize_quotd();

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) die("error opening socket.");

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
     sizeof(serv_addr)) < 0) die("error binding socket.");

  clilen = sizeof(cli_addr);

  while (1) {
    listen(sockfd, 5);

    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) die("error accepting");

    char* quotd = get_quotd(&n);
    n = write(newsockfd, quotd, n);
    free(quotd);
    if (n < 0) die("error writing to socket");
    close(newsockfd);
  }

  close(sockfd);
  cleanup_quotd();
  return 0;
}
