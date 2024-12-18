#include <glib-2.0/glib.h>

static char *m_selfArch;
static char *m_systemVersion;

void *get_system_arch();
void *get_system_version();

char *search_apps(const char *categoryid, const char *orderway, int pageno, int pagesize, const char *searchkeyword, GList **result);