#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <sys/types.h>

#define N_FIELDS 10

void parse_log(char *logfile);
int preg_match(char* src, char* pattern, int field_count);
//char** preg_match(char* src, char* pattern, int field_count);
char** split(char *str);

int main(int argc, char* argv[])
{
	if (argc > 1)
		parse_log(argv[1]);
	else
		printf("Need log file\n");
	return 0;
}

char** split(char* str)
{
	int i = 0;
	char **result;
	char *cell;

	result = malloc(N_FIELDS * sizeof(char *));
	memset(result, 0, N_FIELDS * sizeof(char *));

	cell = strtok(str, " ");

	while (cell != NULL && i < N_FIELDS) {
		result[i++] = cell;
		cell = strtok(NULL, " ");
	}

	return result;
}

void parse_log (char* logfile)
{
	FILE *file;
	int sz;
	char *result_string;
	char** parsed;
//	char** user;
	char* pattern;
	pattern = "([^@]+)@(.*)";
	int i = 0;
	//char* site;
	file = fopen(logfile,"r");
	result_string = NULL;
	while(getline(&result_string, &sz, file) > 0) {
//		printf("str = %d\n", i++);
		parsed = split (result_string);
	//	user = parsed[7];
	//	site = parsed[6];
//		printf("%s\n", site);
 //       	user = 
		preg_match (parsed[7], pattern, 2);
	//	if (user != 1)
	//		printf ("%s\n", user[1]);
		free(result_string);
	//	free(user);
		free(parsed);
		result_string = NULL;
	}
	fclose(file);
}
//char** preg_match(char* src, char* pattern, int field_count)
int preg_match(char* src, char* pattern, int field_count)
{
   int i = 0;
   int res;
   char** f_result;
   int len;
   char result[BUFSIZ];
   char err_buf[BUFSIZ];
   regex_t preg;

   f_result = malloc(field_count * sizeof(char *));
   memset(f_result, 0, field_count * sizeof(char *));

   regmatch_t pmatch[10];

   if( (res = regcomp(&preg, pattern, REG_EXTENDED)) != 0)
   {
      regerror(res, &preg, err_buf, BUFSIZ);
      printf("regcomp: %s\n", err_buf);
      exit(res);
   }

   res = regexec(&preg, src, 10, pmatch, REG_NOTBOL);
   if(res == REG_NOMATCH)
   {
      printf("NO match\n");
      return 1;
   }
   for (i = 0; i <= field_count; i++)
   {
      len = pmatch[i].rm_eo - pmatch[i].rm_so;
      memcpy(result, src + pmatch[i].rm_so, len);
      result[len] = 0;
      f_result[i] = result; 
//      printf("num %d: '%s'\n", i, result);
   }
   printf("fieald1 = %s\n", f_result[0]);
   printf("fieald2 = %s\n", f_result[1]);
   printf("fieald3 = %s\n", f_result[2]);
   regfree(&preg);
//   return f_result;
   return 0;
}

