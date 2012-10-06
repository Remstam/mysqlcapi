/*Created on: 23.05.2012*/

#ifndef COMMON_H_
#define COMMON_H_

#include<mysql/mysql.h>

void print_error(MYSQL *conn, char *message);
int print_dashes(MYSQL_RES *res_set);
int process_result_set(MYSQL *conn, MYSQL_RES *res_set);
int process_query(MYSQL *conn, char *query);

MYSQL *do_connect(char *host_name,
		  char *user_name,
		  char *password,
		  char *db_name,
		  unsigned int port_num,
		  char *socket_name,
		  unsigned int flags);

void do_disconnect(MYSQL *conn);

#endif /* COMMON_H_ */
