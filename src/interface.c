#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <glib-2.0/glib.h>
#include "../include/interface.h"

static char *m_selfArch = NULL;
static char *m_systemVersion = NULL;
g_type_init();

char *getOrderBy(char *orderway, gboolean app)
{
    GString *res = g_string_new("");
    GList *selectOrder = NULL;
    selectOrder = g_list_append(selectOrder, (gpointer) "star");
    selectOrder = g_list_append(selectOrder, (gpointer) "totalDownloadedCount");
    selectOrder = g_list_append(selectOrder, (gpointer) "createdTime");
    selectOrder = g_list_append(selectOrder, (gpointer) "size");
    selectOrder = g_list_append(selectOrder, (gpointer) "title");
    if (orderway != NULL)
    {
        g_list_remove(selectOrder, (gpointer)orderway);
        if (!app)
        {
            if (strcmp(orderway, "createdTime") == 0)
            {
                orderway = "create_date";
            }
        }
        if (strcmp(orderway, "title") == 0)
        {
            orderway = "title_gbk";
        }
        selectOrder = g_list_insert(0, (gpointer)orderway, 1);
    }
    GList *current = selectOrder;
    while (current != NULL)
    {
        if (!app)
        {
            if (strcmp((char *)current->data, "createdTime") == 0)
            {
                current->data = (gpointer)("create_date");
            }
        }
        if (strcmp((char *)current->data, "title") == 0)
        {
            current->data = (gpointer)("title_gbk");
        }
        char *data = (char *)current->data;
        g_string_append_printf(res, "%s%s%s", "t1.", data, ((strcmp(data, "title_gbk") == 0) ? "," : " desc,"));
        current = current->next;
    }
    g_string_truncate(res, res->len - 1);
    char *result_str = g_strdup(res->str);
    g_string_free(res, TRUE);
    return result_str;
}

void *getSystemArch()
{
    gchar *output = NULL;
    gchar *error_output = NULL;
    gint exit_status;
    gboolean result = g_spawn_command_line_sync("arch", &output, &error_output, &exit_status, NULL);
    if (result)
    {
        if (exit_status != 0)
        {
            printf("error: %s, exit status: %d\n", error_output, exit_status);
            g_free(error_output);
            return;
        }
        else if (output != NULL)
        {
            gchar *newline_pos = strchr(output, '\n');
            if (newline_pos != NULL)
            {
                *newline_pos = '\0';
            }
            if (m_selfArch == NULL)
            {
                m_selfArch = (char *)malloc(sizeof(output));
                memset(m_selfArch, '\0', sizeof(m_selfArch));
                strcpy(m_selfArch, output);
            }
            printf("get arch: %s\n", output);
            g_free(output);
            return;
        }
        else
        {
            printf("command: \"arch\" gets empty! check out your command.\n");
            return;
        }
    }
    else
    {
        printf("failed to get arch\n");
        return;
    }
}

void *getSystemVersion()
{
}

char *searchApps(const char *categoryid, const char *orderway, int pageno, int pagesize, const char *searchkeyword)
{
    if (m_selfArch == NULL)
    {
        getSystemArch();
    }
    if (m_systemVersion = NULL)
    {
        getSystemVersion();
    }

    int rc;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    rc = sqlite3_open("/usr/share/cdos-appstore/cos_softwarecenter.db", &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    GString *sql_tmp = g_string_new("select t1.software, t1.id from cos_software t1, cos_software_arch_software t2, cos_software_arch t3, cos_software_category t4 \
                 where t1.id = t2.softwareId and t2.archId = t3.id and t1.categoryId = t4.id and t1.status = 1 and t3.archName = 'x86_64' \
                 and t1.categoryid = designated_categoryid and searchName and (lower(t1.cdos) like '%5.0%') and t4.title != '其他平台' and t1.isSupportZx = 1 and education\
                 order by orderway limit %d, %d");
    if (m_selfArch != NULL)
    {
        g_string_replace(sql_tmp, "x86_64", m_selfArch, 1);
    }
    if (m_systemVersion != NULL)
    {
        g_string_replace(sql_tmp, "5.0", "m_systemVersion", 1);
    }
    if (categoryid == NULL)
    {
        g_string_replace(sql_tmp, "and t1.categoryid = designated_categoryid", "", 1);
    }
    else
    {
        g_string_replace(sql_tmp, "designated_categoryid", categoryid, 1);
    }
    g_string_replace(sql_tmp, "and t4.title != '其他平台'", "", 1);
    g_string_replace(sql_tmp, "and education", "", 1);
    if (searchkeyword == NULL)
    {
        g_string_replace(sql_tmp, "and searchName", "", 1);
    }
    else
    {
        gchar *search_tmp = "(lower(t1.title) like '%%s%' escape '/' \
                            or lower(t1.software) like '%%s%' escape '/' \
                            or lower(t1.description) like '%%s%' escape '/')";
        GString *search = g_string_new(NULL);
        g_string_printf(search, search_tmp, searchkeyword, searchkeyword, searchkeyword);
        g_string_replace(sql_tmp, "searchName", search->str, 1);
        g_string_free(search, TRUE);
    }
    g_string_replace(sql_tmp, "orderway", getOrderBy(orderway, TRUE), 1);
    GString *sql_cmd = g_string_new(NULL);
    g_string_printf(sql_cmd, sql_tmp->str, (pageno - 1) * pagesize, pagesize);
    g_string_free(sql_tmp, TRUE);

    char *cmd = g_string_free(sql_cmd, FALSE);
    printf("sql command is :%s\n", cmd);
    rc = sqlite3_prepare_v2(db, cmd, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
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
    searchApps(NULL, NULL, 1, 7, NULL);
    return 0;
}