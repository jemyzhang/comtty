#include "logging.h"
#include "common.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int create_log(char *logpath) {
  if ((fopen(logpath, "wb")) == NULL) {
    printf("Resource %s busy\n", logpath);
    return -1;
  }
  return 0;
}

static int put_log(char *logpath, char *log, int size) {
  FILE *flog;
  if ((flog = fopen(logpath, "a")) == NULL) {
    return -1;
  } else {
    fwrite(log, size, 1, flog);
    fclose(flog);
  }
  return 0;
}

#define LOG_BLOCK_SIZE 64 * 1024
void *log_to_file(void *data) {
  CTRL_INFO_t *pctrl = (CTRL_INFO_t *)data;
  char *pbuf = (char *)malloc(LOG_BLOCK_SIZE);

  int flags = fcntl(pctrl->log_pipe_fd, F_GETFL);
  fcntl(pctrl->log_pipe_fd, F_SETFL, flags | O_NONBLOCK);

  while (1) {
    int rdsize;
    while ((rdsize = read(pctrl->log_pipe_fd, pbuf, LOG_BLOCK_SIZE)) > 0) {
      if (pctrl->log_switch) {
        put_log(pctrl->log_path, pbuf, rdsize);
      }
    }
    if (pctrl->sig_term) {
      break;
    }
    sleep(1);
  }

  close(pctrl->log_pipe_fd);

  free(pbuf);
  return NULL;
}
