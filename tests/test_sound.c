#include "sound.h"
#include <assert.h>

/* Prototype for internal plugin lookup so we can check whether a driver
 * was found before calling SoundOpen. */
SoundDriver *GetPlugin(const gchar *drivername);

int main(void) {
  SoundInit();

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

  return 0;
}
