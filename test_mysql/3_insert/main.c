#include <stdio.h>
#include <mysql.h>
#include <string.h>


int main(int argc, const char *argv[])
{
    MYSQL           mysql;
    MYSQL_RES       *res = NULL;
    MYSQL_ROW       row;
    char            *query_str = NULL;
    int             rc, i, fields;
    int             rows;

    if (NULL == mysql_init(&mysql)) {               //初始化mysql变量
        printf("mysql_init(): %s\n", mysql_error(&mysql));
        return -1;
    }

    if (NULL == mysql_real_connect(&mysql,         //链接mysql数据库
                "localhost",                       //链接的当地名字
                "root",                            //用户名字
                "root",                            //用户密码
                "RUNOOB",                          //所要链接的数据库
                0,
                NULL,
                0)) {
        printf("mysql_real_connect(): %s\n", mysql_error(&mysql));
        return -1;
    }

    printf("1. Connected MySQL successful! \n");

    query_str = "insert into runoob_tbl values (1,'gaga' ,'justtest', '2015-5-5')";   //插入 SQL 语句
    rc = mysql_real_query(&mysql, query_str, strlen(query_str));     //对数据库执行 SQL 语句
    if (0 != rc) {
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return -1;
    }

    query_str = "delete from runoob_tbl where runoob_id=2";                          //删除 SQL 语句
    rc = mysql_real_query(&mysql, query_str, strlen(query_str));   //对数据库 执行 SQL 语句
    if (0 != rc) {
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return -1;
    }

    query_str = "select * from runoob_tbl";                        //显示 表中数据
    rc = mysql_real_query(&mysql, query_str, strlen(query_str));
    if (0 != rc) {
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return -1;
    }

    res = mysql_store_result(&mysql);                             //获取查询结果
    if (NULL == res) {
         printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
         return -1;
    }

    rows = mysql_num_rows(res);                              
    printf("The total rows is: %d\n", rows);

    fields = mysql_num_fields(res);
    printf("The total fields is: %d\n", fields);

    while ((row = mysql_fetch_row(res))) {
        for (i = 0; i < fields; i++) {
            printf("%s\t", row[i]);
        }
        printf("\n");
    }


    mysql_free_result(res);
    mysql_close(&mysql);

    return 0;
}

