#include "dopewars.h"
#include <assert.h>
#include <glib.h>

/* Stub globals to satisfy AddPlayer dependencies */
int NumDrug = 1;
int NumGun = 1;
int NumCop = 0;
gboolean Server = FALSE;
gboolean UseSocks = FALSE;
price_t StartCash = 0;
price_t StartDebt = 0;
int BaseCoatSize = 0;
struct DATE StartDate = {31, 2, 2023}; /* intentionally invalid */

/* Stub functions required by AddPlayer */
void SetPlayerName(Player *Play, char *Name) { (void)Play; (void)Name; }
void InitAbilities(Player *Play) { (void)Play; }

int main(void) {
    Player *p = g_malloc0(sizeof(Player));
    GSList *list = NULL;
    list = AddPlayer(0, p, list);
    assert(list != NULL);
    assert(g_date_valid(p->date));
    assert(g_date_get_day(p->date) == 5);
    assert(g_date_get_month(p->date) == 1);
    assert(g_date_get_year(p->date) == 1900);

    /* Ensure gameplay can progress beyond early turns */
    for (int i = 0; i < 20; i++) {
        g_date_add_days(p->date, 1);
        p->Turn++;
    }
    assert(p->Turn > 10);
    assert(g_date_valid(p->date));

    g_date_free(p->date);
    g_free(p->Drugs);
    g_free(p->Guns);
    g_free(p);
    return 0;
}
