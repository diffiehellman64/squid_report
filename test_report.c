#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <sys/types.h>

#define N_FIELDS 10

void parse_log(char *logfile);
int preg_match(char* src, char* pattern, int field_count);
char** split(char *str);
char* cut_user(char* str);
char* cut_site(char* site);

struct squid{
	char org[10];
	char user[10];
	char site[25];
};

int
main(int argc, char* argv[])
{
	if (argc > 1)
	{
		printf("%d\n", getFileSize(argv[1]));
		//printf("ok\n");
	//	getch();
		parse_log(argv[1]);
	}
	else
		printf("Need log file\n");
	return 0;
}

char **
split(char* str)
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

char*
cut_user(char* str)
{
	char* result;
	int i;
	for (i = 0; i <= strlen(str); ++i)
	{
		if (str[i] == '@')
			str[i] = '\0';
	}
	result = malloc(strlen(str) * sizeof(char *));
	result = str;
	return result;
}

char*
cut_site(char* site)
{
	char* result;
	char* tmp;
	char* str2;
	int i;
	int point_count = 2;
	for (i = 8; i < strlen(site); ++i)
	{
		if (site[i] == '/' || site[i] == ':')
			site[i] = '\0';
	}
	
	for (i = strlen(site); i >= 0; --i){
		if (site[i] == '.')
			--point_count;
		if (site[i] == '/' || point_count == 0)
		{
			site = site + i + 1; 
			break;
		}
	}
	result = malloc(strlen(site) * sizeof(char *));
	result = site;
	return result;
}
/*
char**
get_sites()
{
	FILE *file;
	int sz;
	char *result_string;
	result_string = NULL;
	file = fopen("monitor_sites.list" ,"r");
	while(getline(&result_string, &sz, file) > 0) {
	{
		printf("%s\n", result_string);
		free(result_string);
		result_string = NULL;
	}
	fclose(file);
}
*/

int getFileSize(char* input)
{
    FILE *file;
    file = fopen(input, "r");
    int fileSizeBytes;
    // set file pointer to end of file
    fseek(file, 0, SEEK_END);
    // get current pointer position in bytes (== file size in bytes)
    fileSizeBytes = ftell(file);
    // return pointer to begin of file (it's very IMPORTANT)
    fseek(file, 0, SEEK_SET);
    fclose(file);
    return fileSizeBytes;
}

/*
void show_progress()
{

}
*/
void
parse_log(char* logfile)
{
	FILE *file;
	int sz;
	char *result_string;
	char** parsed;
	char* user;
	int i = 1;
	char* site;
	file = fopen(logfile,"r");
	result_string = NULL;
//	get_sites();
	while(getline(&result_string, &sz, file) > 0) {
		parsed = split (result_string);
		user = cut_user(parsed[7]);
		site = cut_site(parsed[6]);
//		printf("%d\n", strlen(result_string) * sizeof(char *));
//		printf("%s => %s", user, site);
		free(result_string);
		free(parsed);
		result_string = NULL;
	}
	fclose(file);
}
