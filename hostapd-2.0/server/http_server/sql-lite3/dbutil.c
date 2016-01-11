#include <stdio.h>
#include <string.h>
#include "sqlite3.h"
#include "dbutil.h"

#ifndef DEBUG
#define DEBUG 0
#endif

#define ERROR -1
#define SUCCESS 0

// the "user" database
sqlite3 *db;

// Given a database "db", exec the "sql".
static int Db_exec(sqlite3 *db, const char* sql);
static int Db_open (char *file);

static int Db_open(char *file)
{
  file = file? file: "./db/users.db";
  int ec = sqlite3_open(file, &db);
  if(ec){
    fprintf(stderr
	    , "open db failed: [%s]\n"
	    , sqlite3_errmsg(db));
    sqlite3_close(db);
    return ERROR;
  }
  if(DEBUG)
    printf("open db success!\n");
  return SUCCESS;
}


void Db_init(char *file)
{	
  if(Db_open(file)!=SUCCESS)
    return;

  char *sql = "CREATE TABLE IF NOT EXISTS user(\
			id INTEGER PRIMARY KEY,	 \
                    	name VARCHAR(50),	 \
                    	passwd VARCHAR(50),	 \
                        balance VARCHAR(50)	 \
        	    )";
  
  Db_exec(db, sql);
  sqlite3_close(db);   
  return;
}

int Db_checkUser(const char *name)
{   
  int ec = select_db(name, 0);
  return ec;
}

int Db_checkUserPasswd(const char *name
		       , const char *passwd)
{   
  int ec = select_db(name, passwd);
  return ec;
}

#define DEFAULT_BALANCE 100
int Db_registerUser(const char* u_name
		    , const char* u_passwd)
{ 
  if(Db_open(0)!=SUCCESS){
    fprintf(stderr
	    , "open failed: [%s]\n"
	    , sqlite3_errmsg(db));
    return 0;
  }
  char sql[1024];
  sprintf(sql, "INSERT INTO user(name, passwd, balance) VALUES ('%s', '%s', %s)"
	  , u_name
	  , u_passwd
	  , "0");
  
  Db_exec(db, sql);
  sqlite3_close(db);
  return 1;
}


int select_db(const char *name
	      , const char *passwd)
{
  if(DEBUG)
    printf("name=[%s], passwd=[%s]\n"
	   , name
	   , passwd? passwd: "");
  
  if(Db_open(0)!=SUCCESS){
    printf("open failed![%s]\n"
	   , sqlite3_errmsg(db));
    return 0;
  }    
  char sql[1024];
  if(passwd)
    sprintf(sql, "SELECT * from user WHERE name = '%s' AND passwd= '%s' ", name, passwd);
  else
    sprintf(sql, "SELECT * from user WHERE name = '%s'", name);
  int row, column;
  char **result;
  char *errorMsg;
  int ret = sqlite3_get_table(db, sql, &result, &row, &column, &errorMsg);
  if(ret!=SQLITE_OK){
    printf("Error: rows: [%d]\n", row);
    printf("ErrMsg:[%s]\n", errorMsg);
  }
  else printf ("rows=[%d], cols=[%d]\n"
	       , row
	       , column);
  
  sqlite3_free_table(result);
  //Db_exec(db, sql);
  sqlite3_close(db);
  return row;    
}

int Db_exec(sqlite3 *db, const char *sql)
{
  char *error = 0;
  int ret = sqlite3_exec(db, sql, 0, 0, &error);

  if(ret!=SQLITE_OK){
    printf("Db_exec failed! [%s]: %s\n", sql, error);
    return 0;
  }
  if(DEBUG)
    printf("Db_exec succeed!\n");
  return 1;
}

char * Db_readBalance (const char *name)
{
  if(DEBUG)
    printf("Db_readBalance: name=[%s]\n", name);
  
  if(Db_open(0)!=SUCCESS){
    fprintf(stderr
	    , "open failed![%s]\n"
	   , sqlite3_errmsg(db));
    return 0;
  }
  char sql[1024];
  sprintf(sql, "SELECT balance from user WHERE name = '%s'", name);
  int row, column;
  char **result;
  char *errorMsg;
  int ret = sqlite3_get_table(db, sql, &result, &row, &column, &errorMsg);
  if (ret!=SQLITE_OK){
    fprintf (stderr
	     , "error read balance\n");
    return 0;
  }
  if(DEBUG){
    printf("nrow=[%d], column=[%d]\n", row, column);
    printf ("======%s\n", (result+1)[column-1]);
  }
  char * balance =  ((result+1)[column-1]);
  sqlite3_free_table(result);
  //Db_exec(db, sql);
  sqlite3_close(db);
  return balance;
}
/*wanglei*/
char * db_getSSID (char* value)
{
  
  if(Db_open(0)!=SUCCESS){
    fprintf(stderr
	    , "open failed![%s]\n"
	   , sqlite3_errmsg(db));
    return 0;
  }
  char sql[1024];
  sprintf(sql, "SELECT name from user WHERE balance = '%s'", value);
  int row, column;
  char **result;
  char *errorMsg;
  int ret = sqlite3_get_table(db, sql, &result, &row, &column, &errorMsg);
  if (ret!=SQLITE_OK){
    fprintf (stderr
	     , "error read name\n");
    return NULL;
  }
  char* balance = ((result+1)[column-1]);
  sqlite3_free_table(result);
  sqlite3_close(db);
  return balance;
}
/*wanglei*/
char * db_getPASSWD (char* value)
{
  
  if(Db_open(0)!=SUCCESS){
    fprintf(stderr
	    , "open failed![%s]\n"
	   , sqlite3_errmsg(db));
    return 0;
  }
  char sql[1024];
  sprintf(sql, "SELECT passwd from user WHERE balance = '%s'", value);
  int row, column;
  char **result;
  char *errorMsg;
  int ret = sqlite3_get_table(db, sql, &result, &row, &column, &errorMsg);
  if (ret!=SQLITE_OK){
    fprintf (stderr
	     , "error read passwd\n");
    return NULL;
  }
  char* balance = ((result+1)[column-1]);
  sqlite3_free_table(result);
  sqlite3_close(db);
  return balance;
}




int Db_writeBalance (const char *name
		     , const char *balance)
{
  if(DEBUG)
    printf("Db_writeBalance(): name=[%s]\n"
	   , name);
  
  if(Db_open(0)!=SUCCESS){
    fprintf(stderr
	    , "open failed![%s]\n"
	   , sqlite3_errmsg(db));
    return 0;
  }
  char sql[1024];
  char *errorMsg;
  sprintf(sql, "UPDATE user set balance='%s' where name= '%s'", balance, name);
  int ret = sqlite3_exec(db, sql, 0, 0, &errorMsg);
  if (ret!=SQLITE_OK){
    fprintf (stderr
	     , "%s\n"
	     , "error write balance");
    return 0;
  }
  if(DEBUG){
    printf("Db_writeBalance() succeed\n");
  }
  sqlite3_close(db);
  return 1;
}
	
/*
void main()
{
  
  int t = use_db("zat","123",1);
  if(t)
    printf("Login success*************\n");
  else
    printf("Login fail**************\n");

  t = use_db("sb","123",0);
  if(t)
    printf("user exist*****\n");
  else
    {
      printf("user no exist*************\n");
      register_db("sb", "123");
    }
}
*/
