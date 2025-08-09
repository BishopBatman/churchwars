#include "network.h"
#include <assert.h>
#include <string.h>

static void dummy_cb(NetworkBuffer *NetBuf, gboolean Read, gboolean Write,
                     gboolean Exception, gboolean CallNow) {
  (void)NetBuf; (void)Read; (void)Write; (void)Exception; (void)CallNow;
}

static void reset_connbuf(ConnBuf *buf) {
  if (buf->Data) g_free(buf->Data);
  buf->Data = NULL;
  buf->Length = 0;
  buf->DataPresent = 0;
}

int main(void) {
  NetworkBuffer nb;
  InitNetworkBuffer(&nb, '\n', '\0', NULL);
  SetNetworkBufferCallBack(&nb, dummy_cb, NULL);

  /* Basic credentials */
  reset_connbuf(&nb.negbuf);
  SendSocks5UserPasswd(&nb, "user", "pass");
  assert(nb.negbuf.DataPresent == 3 + 4 + 4);
  unsigned char *b = (unsigned char *)nb.negbuf.Data;
  assert(b[0] == 1);
  assert(b[1] == 4);
  assert(memcmp(b + 2, "user", 4) == 0);
  assert(b[6] == 4);
  assert(memcmp(b + 7, "pass", 4) == 0);

  /* Long credentials of length 255 */
  reset_connbuf(&nb.negbuf);
  char longuser[256];
  char longpass[256];
  memset(longuser, 'u', 255); longuser[255] = '\0';
  memset(longpass, 'p', 255); longpass[255] = '\0';
  SendSocks5UserPasswd(&nb, longuser, longpass);
  assert(nb.negbuf.DataPresent == 3 + 255 + 255);
  b = (unsigned char *)nb.negbuf.Data;
  assert(b[0] == 1);
  assert(b[1] == 255);
  assert(memcmp(b + 2, longuser, 255) == 0);
  assert(b[2 + 255] == 255);
  assert(memcmp(b + 3 + 255, longpass, 255) == 0);

  /* Boundary: credentials too long (256 chars) */
  reset_connbuf(&nb.negbuf);
  char too_long[257];
  memset(too_long, 'x', 256); too_long[256] = '\0';
  SendSocks5UserPasswd(&nb, too_long, "p");
  assert(nb.negbuf.DataPresent == 0);

  /* Boundary: empty credentials */
  reset_connbuf(&nb.negbuf);
  SendSocks5UserPasswd(&nb, "", "p");
  assert(nb.negbuf.DataPresent == 0);

  reset_connbuf(&nb.negbuf);
  return 0;
}
