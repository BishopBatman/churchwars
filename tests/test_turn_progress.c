#include "dopewars.h"
#include <assert.h>
#include <glib.h>
#include <string.h>

/* Stub globals needed by GetDateString */
struct DATE StartDate = {1, 1, 1095};
struct NAMES Names = {0};

int main(void) {
    Player p = {0};
    Names.Date = "Turn %T";
    p.date = g_date_new_dmy(StartDate.day, StartDate.month, StartDate.year);
    GString *out = g_string_new(NULL);

    for (int i = 1; i <= 40; i++) {
        p.Turn = i;
        GetDateString(out, &p);
        char buf[32];
        snprintf(buf, sizeof(buf), "Turn %d", i);
        assert(strcmp(out->str, buf) == 0);
        g_date_add_days(p.date, 1);
    }

    g_string_free(out, TRUE);
    g_date_free(p.date);
    return 0;
}
