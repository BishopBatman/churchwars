#include "sound.h"
#include <assert.h>

/* Prototype for internal plugin lookup so we can check whether a driver
 * was found before calling SoundOpen. */
SoundDriver *GetPlugin(const gchar *drivername);
void AddPlugin(SoundDriver *(*ifunc)(void), void *module);

/* A dummy sound driver whose open routine always fails. */
static gboolean failing_open(void) {
  return FALSE;
}

static SoundDriver *failing_init(void) {
  static SoundDriver drv = { NULL, "failing-driver", failing_open, NULL, NULL };
  return &drv;
}

int main(void) {
  SoundInit();
  /* Sound should start disabled until a driver is opened. */
  assert(IsSoundEnabled() == FALSE);

  /* Opening with no name uses the first available driver, if any. */
  SoundDriver *drv = GetPlugin(NULL);
  SoundOpen(NULL);
  assert(IsSoundEnabled() == (drv != NULL));

  /* Closing should always disable sound. */
  SoundClose();
  assert(IsSoundEnabled() == FALSE);

  /* Explicitly requesting a bad driver leaves sound disabled. */
  SoundOpen("nonexistent-driver");
  assert(IsSoundEnabled() == FALSE);

  /* Register a driver that fails to open and ensure sound stays disabled. */
  SoundInit();
  assert(IsSoundEnabled() == FALSE);
  AddPlugin(failing_init, NULL);
  SoundOpen("failing-driver");
  assert(IsSoundEnabled() == FALSE);

  return 0;
}
