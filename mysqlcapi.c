/*Created on: 23.05.2012*/

#include <stdio.h>
#include <mysql/mysql.h>
#include "mysqlcapi.h"

void print_error(MYSQL *conn, char *message)
{
 fprintf(stderr,"%s\n",message);
 if(conn!=NULL)
 {
  fprintf(stderr,"Error %u (%s)\n", mysql_errno(conn),mysql_error(conn));
 }
}

int print_dashes(MYSQL_RES *res_set)
{
  MYSQL_FIELD *field;
  unsigned int i,j;

  mysql_field_seek(res_set,0);
  fputc('+',stdout);

  for(i=0;i<mysql_num_fields(res_set);i++)
  {
    field = mysql_fetch_field(res_set);
    for(j=0;j<field->max_length+2;j++)
      fputc('+',stdout);
  }
  fputc('\n',stdout);
  return 0;
}

int process_result_set (MYSQL *conn, MYSQL_RES *res_set)
{
  MYSQL_FIELD *field;
  MYSQL_ROW row;
  unsigned int i, col_len;

  mysql_field_seek(res_set,0);
  for(i=0;i<mysql_num_fields(res_set);i++)
  {
    field = mysql_fetch_field(res_set);
    col_len = strlen(field->name);

    if(col_len<field->max_length)
      col_len = field->max_length;

    if(col_len<4 && IS_NOT_NULL(field->flags))
      col_len = 4;

    field->max_length = col_len;
  }

  print_dashes(res_set);

  fputc('|',stdout);
  mysql_field_seek(res_set,0);
  for(i=0;i<mysql_num_fields(res_set);i++)
  {
    field = mysql_fetch_field(res_set);
    printf("%-*s |",field->max_length, field->name);
  }

  fputc('\n',stdout);
  print_dashes(res_set);

  while((row = mysql_fetch_row(res_set))!=NULL)
  {
    mysql_field_seek(res_set,0);
    fputc('|',stdout);
    for(i=0;i<mysql_num_fields(res_set);i++)
    {
      field = mysql_fetch_field(res_set);
      if(row[i]==NULL)
	printf("%-*s |", field->max_length, "NULL");
      else if(IS_NUM(field->type))
	printf("%*s ", field->max_length, row[i]);
      else
	printf("%-*s |",field->max_length, row[i]);
    }
    fputc('\n',stdout);
  }
  print_dashes(res_set);
  printf("%lu rows affected\n", (unsigned long)mysql_num_rows(res_set));
  return 0;
}

int process_query(MYSQL *conn, char *query)
{
  MYSQL_RES *res_set;

  if(mysql_query(conn,query)!=0)
  {
    print_error(conn,"Сбой process_query()");
    return 1;
  }

  res_set = mysql_store_result(conn);
  if(res_set==NULL)
  {
    if(mysql_field_count(conn)>0)
      print_error(conn,"Невозможно обработать результирующий набор");
    else
      printf("%lu rows affected\n", (unsigned long)mysql_affected_rows(conn));
  }
  else
  {
    process_result_set(conn,res_set);
    mysql_free_result(res_set);
  }
  return 0;
}

MYSQL *do_connect(char *host_name,
		   char *user_name,
		   char *password,
		   char *db_name,
		   unsigned int port_num,
		   char *socket_name,
		   unsigned int flags)
{
 MYSQL *conn;
 conn = mysql_init(NULL);

 if(conn==NULL)
 {
  print_error(NULL,"Сбой mysql_init() (вероятно, не хватает памяти)");
  return (NULL);
 }

#if defined(MYSQL_VERSION_ID) && MYSQL_VERSION_ID>=32200 /*3.22 и выше*/
 if(mysql_real_connect(conn,
		       host_name,
		       user_name,
		       password,
		       db_name,
		       port_num,
		       socket_name,
		       flags)==NULL)
 {
  print_error(conn,"Сбой mysql_real_connect()");
  return (NULL);
 }
#else /*3.22*/
 if(mysql_real_connect(conn,
		       host_name,
		       user_name,
		       password,
		       port_num,
		       socket_name,
		       flags)==NULL)
 {
  print_error(conn,"Сбой mysql_real_connect()");
  return (NULL);
 }

 if(db_name != NULL)
 {
  if(mysql_select_db(conn,name)!=0)
  {
   print_error(conn,"Сбой mysql_select_db()");
   mysql_close(conn);
   return (NULL);
  }
 }
#endif

 return conn;
}

void do_disconnect(MYSQL *conn)
{
 mysql_close(conn);
}
