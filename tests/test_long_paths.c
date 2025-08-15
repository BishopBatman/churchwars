#include "configfile.h"
#include <glib.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(void) {
    char templ[] = "/tmp/cwtestXXXXXX";
    char *base = mkdtemp(templ);
    assert(base);

    gchar component[201];
    memset(component, 'a', 200);
    component[200] = '\0';

    GString *path = g_string_new(base);
    for (int i = 0; i < 10; i++) {
        g_string_append_c(path, '/');
        g_string_append(path, component);
    }
    g_string_append(path, "/score.sco");
    assert(ensure_scorefile_ready(path->str) == 0);

    GString *bad = g_string_new(base);
    g_string_append_c(bad, '/');
    for (int i = 0; i < 300; i++)
        g_string_append_c(bad, 'b');
    g_string_append(bad, "/score.sco");
    errno = 0;
    assert(ensure_scorefile_ready(bad->str) == -1);
    assert(errno == ENAMETOOLONG);

    GString *cmd = g_string_new(NULL);
    g_string_printf(cmd, "rm -rf %s", base);
    system(cmd->str);

    g_string_free(cmd, TRUE);
    g_string_free(bad, TRUE);
    g_string_free(path, TRUE);
    return 0;
}
