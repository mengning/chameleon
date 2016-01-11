#ifndef DBUTIL_H
#define DBUTIL_H

#include "sqlite3.h"

// Init the database database "file".
// if file==0, init a default dabase. 
// If the data base has not been opened,
// open it, or else it remained opened.
void Db_init (char *file);
int Db_checkUser(const char *username);
// Check whether or not a user is valid.
int Db_checkUserPasswd(const char *username
		       , const char *password);
int Db_registerUser (const char *username
		     , const char *password);
char * Db_readBalance (const char *username);
int  Db_writeBalance (const char *username
		     ,const char* balance);
char * db_getSSID(char *value);
char * db_getPASSWD(char * value);
#endif
