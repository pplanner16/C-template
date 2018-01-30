#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

/*Macros for constants*/
#define INIT_BUF_SIZE 10000

/*Function-like macros*/
#define CAT(dest, source, logbuf) do { if ((rc = str_cat(dest, source, \
        logbuf)) != SUCCESS) return rc; } while(0)
#define TESTCASE(func, logbuf, testcount) do { if ((rc = func(logbuf)) !=  \
    SUCCESS) return rc; testcount++; } while(0) 

/*Message related macros*/
#define ULONG_MAX_LEN (sizeof(unsigned long)*CHAR_BIT/3 +2)
#define ERR_MES_STR_COPY "string copy failed : source length %lu > " \
  "dest allocated space %lu\n"
#define ERR_MES_STR_COPY_LEN (sizeof ERR_MES_STR_COPY + 2*ULONG_MAX_LEN)
#define ERR_MES_STR_CAT "string cat failed : source length %lu > " \
  "dest free space %lu\n"
#define ERR_MES_STR_CAT_LEN (sizeof ERR_MES_STR_CAT + 2*ULONG_MAX_LEN)


/*Return codes*/
typedef enum {
  SUCCESS,
  ERR_STR_COPY,
  ERR_STR_CAT,
  ERR_STR_MALLOC,
  ERR_RC_NOT_SET } td_rc;

/*Dynamically allocated string with allocated size */
struct strbuff{
  size_t size;
  char *str;
};

/*Function prototypes*/

/*Convenience wrapper around malloc with optional log parameter*/
td_rc str_malloc(struct strbuff *newbuf, size_t bufsize, 
    struct strbuff *logbuf);
/*Safer replacement for strcpy with optional log parameter*/
td_rc str_copy(struct strbuff *dest, char *source, struct strbuff *logbuf);
/*Safer replacement for strcat with optional log parameter*/
td_rc str_cat(struct strbuff *dest, char *source, struct strbuff *logbuf);
/*Test runner*/
td_rc test_all(struct strbuff *logbuf);
/*Test cases*/
td_rc test_str_malloc(struct strbuff *logbuf);
td_rc test_str_copy(struct strbuff *logbuf);
td_rc test_str_cat(struct strbuff *logbuf);

/*Output HTTP header and log messages*/
int main(int argc, char** argv){
  td_rc rc = ERR_RC_NOT_SET;
  struct strbuff logbuf;
  printf("%lu %lu\n",ULONG_MAX_LEN, ERR_MES_STR_CAT_LEN);
  if ((rc = str_malloc(&logbuf, INIT_BUF_SIZE, NULL)) != SUCCESS)
    return rc;
  if ((rc = str_copy(&logbuf, "content-type: text/html\n\n", NULL)) 
      == SUCCESS) {
    if (argc > 1)
      if (strcmp(argv[1], "test") == 0)
        /*If the first parameter is "test", run all test cases*/
        rc = test_all(&logbuf);
    printf("%s", logbuf.str);
  }
  free(logbuf.str);
  return rc;
}

td_rc str_malloc(struct strbuff *newbuf, size_t bufsize, 
    struct strbuff *logbuf){
  td_rc rc = ERR_RC_NOT_SET;
  newbuf->str = malloc(sizeof *newbuf->str * bufsize);
  if (!newbuf->str) {
    /*If malloc fails, write error to log if supplied
      otherwise write to standard output*/
    char errmes[100];
    sprintf(errmes, "malloc failed : size requested %lu \n", 
        (unsigned long)bufsize); 
    if (!logbuf)
      printf("%s", errmes);
    else CAT(logbuf, errmes, NULL);
    return ERR_STR_MALLOC;
  }
  /*Terminate the new empty string*/
  *newbuf->str = '\0';
  newbuf->size = bufsize;
  return SUCCESS;
} 

td_rc str_copy(struct strbuff *dest, char *source, struct strbuff *logbuf){
  size_t sourcelen = strlen(source);
  td_rc rc = ERR_RC_NOT_SET;
  /**dest should have been initialized with str_malloc*/
  if (sourcelen > dest->size) {
    /*If if the source string is longer than the destion buffer, 
     * write error to log if supplied otherwise write to standard output*/
    char errmes[ERR_MES_STR_COPY_LEN];
    sprintf(errmes, ERR_MES_STR_COPY, (unsigned long)sourcelen, 
        (unsigned long)dest->size); 
    if (!logbuf)
      printf("%s", errmes);
    else CAT(logbuf, errmes, NULL);
    return ERR_STR_COPY;
  }
  memcpy(dest->str, source, sourcelen+1);
  return SUCCESS;
} 

td_rc str_cat(struct strbuff *dest, char *source, struct strbuff *logbuf){
  td_rc rc = ERR_RC_NOT_SET;
  /**dest should have been initialized with str_malloc*/
  size_t destlen = strlen(dest->str);
  size_t destfree = dest->size - destlen;
  size_t sourcelen = strlen(source);
  if (sourcelen > destfree) {
    /*If the source string is longer than the free space in the 
     * destination buffer, write error to log if supplied otherwise write 
     * to standard output*/
    char errmes[ERR_MES_STR_CAT_LEN];
    sprintf(errmes, ERR_MES_STR_CAT, (unsigned long)sourcelen, 
        (unsigned long)destfree); 
    rc = ERR_STR_CAT;
    if (!logbuf)
      printf("%s", errmes);
    else 
      rc = str_cat(logbuf, errmes, NULL);
  } else { 
    memcpy(dest->str+destlen, source, sourcelen+1);
    rc = SUCCESS;
  }
  return rc;
} 

td_rc test_all(struct strbuff *logbuf){
  td_rc rc = ERR_RC_NOT_SET;
  int testcount = 0;
  char rc_str[20];
  /*If a test case fails, stop further processing and output the log, 
   * otherwise output a test summary*/
  TESTCASE(test_str_malloc, logbuf, testcount);
  TESTCASE(test_str_copy, logbuf, testcount);
  TESTCASE(test_str_cat, logbuf, testcount);
  sprintf(rc_str, "%d", testcount);
  CAT(logbuf, "\n", NULL);
  CAT(logbuf, rc_str, NULL);
  CAT(logbuf, " tests completed successfully\n", NULL);
  return SUCCESS;
}

td_rc test_str_malloc(struct strbuff* logbuf){
  td_rc rc = ERR_RC_NOT_SET;
  struct strbuff teststr;
  if ((rc = str_malloc(&teststr, 5, logbuf)) != 0)
    return rc;
  if (teststr.size == 5)
    rc = SUCCESS;
  else rc = ERR_STR_MALLOC;
  free(teststr.str);
  return rc;
}

td_rc test_str_copy(struct strbuff* logbuf){
  td_rc rc = ERR_RC_NOT_SET;
  struct strbuff teststr;
  if ((rc = str_malloc(&teststr, 4, logbuf)) != 0)
    return rc;
  if ((rc = str_copy(&teststr, "foo", logbuf)) == SUCCESS) {
    if (strcmp(teststr.str, "foo")) 
      rc = ERR_STR_COPY;
    else {
      if ((rc = str_copy(&teststr, "bar", logbuf)) == SUCCESS) {
        if (strcmp(teststr.str, "bar")) 
          rc = ERR_STR_COPY;
      }
    }
  }
  free(teststr.str);
  return rc;
}

td_rc test_str_cat(struct strbuff* logbuf){
  td_rc rc = ERR_RC_NOT_SET;
  struct strbuff teststr;
  if ((rc = str_malloc(&teststr, 7, logbuf)) != SUCCESS)
    return rc;
  if ((rc = str_cat(&teststr, "foo", logbuf)) == SUCCESS) {
    if (strcmp(teststr.str, "foo")) 
      rc = ERR_STR_CAT;
    else {
      if ((rc = str_cat(&teststr, "bar", logbuf)) == SUCCESS) {
        if (strcmp(teststr.str, "foobar")) 
          rc = ERR_STR_CAT;
      }
    }
  }
  free(teststr.str);
  return rc;
}

