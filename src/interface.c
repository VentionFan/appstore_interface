#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <glib-2.0/glib.h>
#include "../include/interface.h"

static char *m_selfArch = NULL;
static char *m_systemVersion = NULL;
g_type_init();

char *getSystemArch()
{
    char *arch = NULL;
    gchar *output = NULL;
    gchar *error_output = NULL;
    gint exit_status;
    gboolean result = g_spawn_command_line_sync("arch", &output, &error_output, &exit_status, NULL);
    if (result)
    {
        if (output != NULL)
        {
            gchar *newline_pos = strchr(output, '\n');
            if (newline_pos != NULL)
            {
                *newline_pos = '\0';
            }
            arch = output;
            printf("get arch: %s\n", output);
            g_free(output);
        }
        if (exit_status != 0)
        {
            printf("error: %s, exit status: %d\n", error_output, exit_status);
            g_free(error_output);
        }
    }
    else
    {
        printf("failed to get arch\n");
    }
    return arch;
}

char *searchApps(char *appName)
{
    if (m_selfArch == NULL)
    {
        m_selfArch = getSystemArch();
    }
    int rc;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    rc = sqlite3_open("/usr/share/cdos-appstore/cos_softwarecenter.db", &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    GString *sql = g_string_new("select t1.software, t1.id from cos_software t1, cos_software_arch_software t2, cos_software_arch t3, cos_software_category t4 \
                 where t1.id = t2.softwareId and t2.archId = t3.id and t1.categoryId = t4.id and t1.status = 1 and t3.archName = 'x86_64' \
                 and (lower(t1.cdos) like '%5.0%') and t1.isSupportZx = 1\
                 order by t1.star desc limit 0, 7");
    g_string_replace(sql, "x86_64", m_selfArch, 1);
    //     g_string_replace(sql, "5.0", "m_systemVersion", 1);

    char *cmd = g_string_free(sql, FALSE);
    rc = sqlite3_prepare_v2(db, cmd, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    // 在这里进行数据库操作，如创建表、插入数据等
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const char *software = (const char *)sqlite3_column_text(stmt, 0);
        const char *id = (const char *)sqlite3_column_text(stmt, 1);
        printf("ID: %s, Software: %s\n", id, software);
    }
    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    free(cmd);
    sqlite3_close(db);
    return NULL;
}

int main()
{
    searchApps("");
    return 0;
}