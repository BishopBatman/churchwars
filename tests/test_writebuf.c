#include "network.h"
#include <assert.h>
#include <string.h>

static void dummy_cb(NetworkBuffer *NetBuf, gboolean Read, gboolean Write,
                     gboolean Exception, gboolean CallNow) {
  (void)NetBuf; (void)Read; (void)Write; (void)Exception; (void)CallNow;
}

int main(void) {
  NetworkBuffer nb;
  InitNetworkBuffer(&nb, '\n', '\0', NULL);
  SetNetworkBufferCallBack(&nb, dummy_cb, NULL);

  char *longmsg = g_malloc(MAXWRITEBUF + 1);
  memset(longmsg, 'x', MAXWRITEBUF);
  longmsg[MAXWRITEBUF] = '\0';
  gboolean ok = QueueMessageForSend(&nb, longmsg);
  assert(ok == FALSE);
  assert(nb.WriteBuf.DataPresent == 0);
  g_free(longmsg);

  char chunk[1024];
  memset(chunk, 'y', sizeof(chunk)-1);
  chunk[sizeof(chunk)-1] = '\0';
  for (int i = 0; i < 50; i++) {
    ok = QueueMessageForSend(&nb, chunk);
    assert(ok == TRUE);
  }
  assert(nb.WriteBuf.DataPresent < MAXWRITEBUF);
  g_free(nb.WriteBuf.Data);
  return 0;
}
