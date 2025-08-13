#include "serverside.h"
#include "message.h"
#include "dopewars.h"
#include <assert.h>
#include <glib.h>
#include <string.h>
#include <stdarg.h>

#define _(x) (x)

/* Stub globals */
int NumDrug = 1;
int NumGun = 0;
int NumCop = 0;
gboolean Sanitized = TRUE;
struct LOCATION Location[1];
struct GUN Gun[1];
struct NAMES Names = {0};

/* Stub functions */
void SendPlayerData(Player *From) { (void)From; }
void CopsAttackPlayer(Player *From) { (void)From; }
int TotalGunsCarried(Player *From) { (void)From; return 0; }
void GainBitch(Player *From) { (void)From; }
int brandom(int a, int b) { (void)b; return a; }
gchar *dpg_strdup_printf(gchar *format, ...) {
  va_list ap;
  va_start(ap, format);
  gchar *res = g_strdup_vprintf(format, ap);
  va_end(ap);
  return res;
}
void SendPrintMessage(Player *From, AICode AI, Player *To, char *Data) {
  (void)From; (void)AI; (void)To; g_free(Data);
}

/* Minimal helpers from message.c */
gchar *GetNextWord(gchar **Data, gchar *Default) {
  gchar *Word = *Data;
  if (!Word || *Word == '\0')
    return Default;
  while (**Data != '\0' && **Data != '^')
    (*Data)++;
  if (**Data != '\0') {
    **Data = '\0';
    (*Data)++;
  }
  return Word;
}

int GetNextInt(gchar **Data, int Default) {
  gchar *Word = GetNextWord(Data, NULL);
  if (Word) {
    char *endptr;
    long val = strtol(Word, &endptr, 10);
    if (*endptr == '\0' && val >= 0 && val <= G_MAXINT)
      return (int)val;
  }
  return Default;
}

int GetNextSignedInt(gchar **Data, int Default) {
  gchar *Word = GetNextWord(Data, NULL);
  if (Word) {
    char *endptr;
    long val = strtol(Word, &endptr, 10);
    if (*endptr == '\0' && val >= G_MININT && val <= G_MAXINT)
      return (int)val;
  }
  return Default;
}

int main(void) {
  Player p;
  memset(&p, 0, sizeof(p));
  p.Drugs = g_malloc0(sizeof(Inventory) * NumDrug);
  p.Drugs[0].Carried = 5;
  p.Drugs[0].Price = 10;
  p.Drugs[0].TotalValue = p.Drugs[0].Carried * p.Drugs[0].Price;
  p.CoatSize = 10;
  p.Cash = 100;

  char msg[] = "drug^0^-2";
  assert(BuyObject(&p, msg));
  assert(p.Drugs[0].Carried == 3);
  assert(p.Cash == 120);
  assert(p.CoatSize == 12);

  g_free(p.Drugs);
  return 0;
}
