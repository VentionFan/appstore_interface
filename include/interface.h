static char *m_selfArch;
static char *m_systemVersion;

void *getSystemArch();
void *getSystemVersion();

char *searchApps(const char *categoryid, const char *orderway, int pageno, int pagesize, const char *searchkeyword);