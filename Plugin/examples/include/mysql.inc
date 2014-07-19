/*
 * MySQL functions
 *
 * (c) Copyright 2010 - dJeyL
 * This file is provided as is (no warranties).
 *
 */

#if defined _mysql_included
  #endinput
#endif
#define _mysql_included

/* Opens connection. If already such exists then that will be used.
 * Function returns sql id to use with other sql natives.
 * Host can be plain ip or with port separated with ':' char. */
native mysql_connect(host[], user[], pass[], dbname[], error[], maxlength, reconnect = 0);

/* Clears query (sql) and closes connection (if any other plugin doesn't use it). */
native mysql_close(sql);

/* Uses an existing connection (sql) to perform a new query (query) (might close previous query if any). */
native mysql_query(sql, query[], {Float,_}:... );

/* Prepares next row of current query (sql) for read access ; returns the number of the row, 0 at end. */
native mysql_nextrow(sql);

/* Gets a field by number.  Returns 0 on failure.
 * Although internally fields always start from 0,
 * This function takes fieldnum starting from 1.
 * No extra params: returns int
 * One extra param: returns Float: byref
 * Two extra param: Stores string with length */
native mysql_getfield(sql, fieldnum, {Float,_}:... );

/* Gets a field by name.  Returns 0 on failure.
 * No extra params: returns int
 * One extra param: returns Float: byref
 * Two extra param: Stores string with length */
native mysql_getresult(sql, field[], {Float,_}:... );

/* Gets the ID generated from the previous INSERT operation. */
native mysql_insert_id(sql);

/* Gets number of affected rows in previous MySQL operation. */
native mysql_affected_rows(sql);

/* Returns the number of fields/colums in a result set. */
native mysql_num_fields(sql);

/* Returns the number of rows returned from a query. */
native mysql_num_rows(sql);

/* Retrieves the name of a field/column in a result set.
 * Columns are numbered 1 to n. */
native mysql_field_name(sql, field, name[], length);

/* Stores last error of current query/connection (sql) in (dest) with (maxlength) characters maximum. */
native mysql_error(sql, dest[], maxlength);
