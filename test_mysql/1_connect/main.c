#include <stdio.h>
#include <mysql.h>

int main(int argc, const char *argv[])
{
    MYSQL   mysql;

    if (NULL == mysql_init(&mysql)) {
        printf("mysql_init(): %s\n",mysql_error(&mysql));
        return -1;
    }

    if (NULL == mysql_real_connect(&mysql,
                "localhost",
                "root",
                "root",
                "RUNOOB",
                0,
                NULL,
                0)) {
        printf("mysql_real_connect(): %s\n", mysql_error(&mysql));
        return -1;
    }

    printf("Connected MySQL successful! \n");

    mysql_close(&mysql);
    return 0;
}
