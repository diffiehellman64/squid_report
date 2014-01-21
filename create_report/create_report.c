#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>

#include "hash.h"
#include "users_request.h"

#define N_FIELDS 10
#define N_MONITOR_SITES 30

//black magic  Хахаха...
#define TO_STR(arg) _TO_STR(arg)
#define _TO_STR(arg) #arg
//end of black magic

#define	TIME_MAXLEN 128
#define	IP_MAXLEN  16
#define	HEAD_ST_MAXLEN 128
#define	METHOD_MAXLEN 128
#define	URI_MAXLEN 8192
#define	USERNAME_MAXLEN 128
#define ORGNAME_MAXLEN 128

struct log_entry {
	int time;
	int elaps;
	char ipaddr[IP_MAXLEN];
	char head_st[HEAD_ST_MAXLEN];
	int len;
	char method[METHOD_MAXLEN];
	char uri[URI_MAXLEN];
	char username[USERNAME_MAXLEN];
};

void parse_log(FILE *fp);
int read_record(FILE *fp, struct log_entry *entry);

char *chop_uname(char *uname);
char *chop_domain(char *uri);
//diff added
char *cut_site(char *site);
char **getsites(char* filename);

void
usage_and_die(char *pname)
{
	printf("%s logfile1 [logfile2 ...]\n", pname);
	exit(1);
}

int
main(int argc, char* argv[])
{
	FILE *fp;

	if (argc < 2)
		usage_and_die(argv[0]);

	argc--;
	argv++;

	while (argc--) {
		fp = fopen(argv[0], "r");
		parse_log(fp);
		argv++;
	}

	return 0;
}

// Функция должна возвращать массив, в котором содежится перечень сайтов
// для мониторинга.
// В дальнейшем посещения этих сайтов будут считаться, а остальные сайты будут
// переименовываться в other и считаться как other.

char **
getsites(char* filename) {
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char** result;
	int i = 0;
	result = malloc(N_MONITOR_SITES * sizeof(char*));
	fp = fopen(filename, "r");
	if (fp == NULL){
		printf("Cann`t open list of sites for monitoring\n");
		exit(1);
	}
	while ((read = getline(&line, &len, fp)) != -1) {
		//printf("%s", line);
		result[i++] = line;
	}
	fclose(fp);
	free(line);
	return result;
}

void
parse_log(FILE *fp)
{
/*	char** sites = getsites("monitor_sites.list");
	int i = 0;

	for (i = 0; i <= N_MONITOR_SITES; ++i){
		printf("%s\n", sites[i]);
	}
	free (sites);
*/
	user_table_t *table;

	struct log_entry entry;
	
	
	table = user_table_new();

	while (read_record(fp, &entry) == 0) {
//		printf ("%d\n", i++);
//		user_table_add_entry(table, chop_uname(entry.username), chop_domain(entry.uri));
//diff added
		user_table_add_entry(table, chop_uname(entry.username), cut_site(entry.uri));
	}

	user_table_list(table);

	user_table_del(&table);
}

int
read_record(FILE *fp, struct log_entry *entry)
{
	int code;
	int ret;
	int seconds;

	ret = fscanf(fp, "%d.%d %d"
	    "%" TO_STR(IP_MAXLEN) "s "
	    "%" TO_STR(HEAD_ST_MAXLEN) "s "
	    "%d "
	    "%" TO_STR(METHOD_MAXLEN) "s "
	    "%" TO_STR(URI_MAXLEN) "s "
	    "%" TO_STR(USERNAME_MAXLEN) "s ",
	    &entry->time, &seconds, &entry->elaps,
	    entry->ipaddr, entry->head_st, &entry->len,
	    entry->method, entry->uri, entry->username
	    );

	//skip to next line
	while ((code = fgetc(fp)) != '\n') {
		if (code == EOF)
			return -1;
	}

	if (ret < 2)
		return 1;

	return 0;
}

/*
 * Some [maybe] usefull functions
*/
char *
chop_uname(char *uname)
{
	strtok(uname, "@");

	return uname;
}

//Для пользователя у меня вот эта функция, но я не знаю быстрее ли она чем chop_uname
/*
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
*/
//FIXME maybe need to rewrite me
char *
chop_domain(char *uri)
{
	int d_s, d_e;
	char *p;

	d_s = d_e = 0;

	if (strncmp(uri, "http://", 7) == 0)
		d_s = 7;

	if (strncmp(uri, "https://", 8) == 0)
		d_s = 8;
	
	p = strchr(uri + d_s, '/');
	if (p == NULL)
		d_e = strlen(uri);
	else
		d_e = p - uri;

	memmove(uri, uri + d_s, d_e - d_s);
	uri[d_e - d_s] = 0;

	return uri;
}

//Я написал такую функцию по отчленению домена
//Мне нужно только домены 2-го уровня, т.е. не sdfsdf.vk.com, а vk.com
char*
cut_site(char* site) {
//        char* result;
        int i;
        int point_count = 2;
        for (i = 8; i < strlen(site); ++i)
        {
                if (site[i] == '/' || site[i] == ':')
                        site[i] = '\0';
        }
        for (i = strlen(site); i >= 0; --i){
                if (site[i] == '.')
                {
                        if(!(isdigit(site[i + 1]) && point_count == 2))
                        {
                                --point_count;
                        }
                }
                if (site[i] == '/' || point_count == 0)
                {
                        site = site + i + 1; 
                        break;
                }
        }
//        result = malloc(strlen(site) * sizeof(char *));
//        result = site;
//        return result;
//	memmove(site, site + d_s, d_e - d_s);
	return site;
}
