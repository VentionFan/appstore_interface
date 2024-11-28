#include <stdio.h>
#include <sqlite3.h>
#include <glib-2.0/glib.h>
#include "../include/interface.h"

int main() {    
    sqlite3 *db;
    int rc;
    sqlite3_stmt *stmt;
    // 打开或创建数据库文件
    rc = sqlite3_open("/usr/share/cdos-appstore/cos_softwarecenter.db", &db);
    if (rc!= SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    GString *sql = g_string_new("select t1.software, t1.id from cos_software t1, cos_software_arch_software t2, cos_software_arch t3, cos_software_category t4 \
                 where t1.id = t2.softwareId and t2.archId = t3.id and t1.categoryId = t4.id and t1.status = 1 and t3.archName = 'x86_64' \
                 and (lower(t1.cdos) like '%5.0%') and t1.isSupportZx = 1\
                 order by t1.star desc limit 0, 7");    
    GString *sql2 = g_string_new("select t1.software from cos_software t1, cos_software_arch_software t2, cos_software_arch t3, cos_software_category t4 \
                where t1.id = t2.softwareId and t2.archId = t3.id and t1.categoryId = t4.id and t1.status = 1 and t3.archName = 'x86_64' \
                and t1.categoryId = ?  and searchName and (lower(t1.cdos) like '%5.0%') and t4.title != '其他平台' and t1.isSupportZx = 1 and education \
                order by orderWay limit ?, ?");    
    // g_string_replace(sql, "x86_64", m_selfArch, 1);
    // g_string_replace(sql, "5.0", m_systemVersion, 1);

    char *str = g_string_free(sql, FALSE);

    rc = sqlite3_prepare_v2(db, str, -1, &stmt, NULL);
    if (rc!= SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }    
    // 在这里进行数据库操作，如创建表、插入数据等
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // 假设students表有三列：id（整数）、name（文本）、age（整数）
        // int id = sqlite3_column_int(stmt, 0);
        // const char *name = (const char *)sqlite3_column_text(stmt, 1);
        const char *software = (const char *)sqlite3_column_text(stmt, 0);
        const char *id = (const char *)sqlite3_column_text(stmt, 1);
        printf("ID: %s, Software: %s\n", id, software);
    }
    if (rc!= SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    free(str);
    sqlite3_close(db);
    return 0;
}