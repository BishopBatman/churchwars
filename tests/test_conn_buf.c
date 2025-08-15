#include "network.h"
#include <assert.h>
#include <string.h>

static void reset_connbuf(ConnBuf *buf) {
  if (buf->Data) g_free(buf->Data);
  buf->Data = NULL;
  buf->Length = 0;
  buf->DataPresent = 0;
}

int main(void) {
  ConnBuf buf;
  reset_connbuf(&buf);

  /* Negative DataPresent should be clamped to zero */
  buf.DataPresent = -5;
  gchar *ptr = ExpandWriteBuffer(&buf, 16, NULL);
  assert(ptr != NULL);
  assert(buf.DataPresent == 0);
  reset_connbuf(&buf);

  /* Boundary: allow growth up to MAXWRITEBUF */
  const size_t max = 65536; /* MAXWRITEBUF in network.c */
  buf.Data = g_malloc(max);
  buf.Length = max;
  buf.DataPresent = max - 8;
  ptr = ExpandWriteBuffer(&buf, 8, NULL);
  assert(ptr == buf.Data + (max - 8));

  /* Exceeding MAXWRITEBUF should fail */
  assert(ExpandWriteBuffer(&buf, 9, NULL) == NULL);

  reset_connbuf(&buf);
  return 0;
}
