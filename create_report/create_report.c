#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <sys/types.h>

#include "hash.h"
#include "log.h"
#include "macros.h"
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
#define SH_MAXLEN 128
#define TYPE_MAXLEN 128
#define USERAGENT_MAXLEN 512
#define REFERER_MAXLEN 8192

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

void parse_log(FILE *fp, char *csvfile, char *monitor);
int read_record(FILE *fp, struct log_entry *entry);

char *chop_uname(char *uname);
char *chop_domain(char *uri);
char *chop_lvl2_domain(char *domname);
//diff added
char *cut_site(char *site);
char **getsites(char* filename);
int exist_elem (char *elem, char **array, int elem_count);
int count_lines (FILE *fp);
void progress(int global, int curent);

void
usage_and_die(char *pname)
{
	printf("%s logfile1 [logfile2 ...]\n", pname);
	exit(1);
}

//Напиши обработчик опций чтобы можно было включать/отключать болтливый режим,
//выбирать формат вывода, сейчас всё встроено в код что не очень хорошо.
#define N_LINES 10000
int is_verbose = 0;

int
main(int argc, char* argv[])
{
	int rez=0;
	FILE *fp;
	char *logfile;
	char *csvfile = NULL;
	char *monitor = NULL;
	
	while ( (rez = getopt(argc,argv,"l:o:m:v")) != -1){
		switch (rez){
			case 'l': 
				logfile = optarg;	
				break;
			case 'o': 
				csvfile = optarg;
				break;
			case 'm':
				monitor = optarg;
				break;
			case 'v': 
				printf("VERBOSE MOD!\n");
				is_verbose = 1; 
				break;
			case '?': 
				printf("Error found !\n");
				break;
        	};
	};

        fp = fopen(logfile, "r");
	if (fp == NULL) {
		warning("Can't open logfile\n");
		exit(1);
	}

	if (csvfile == NULL) {
		csvfile = "test.csv";
	}

	if (monitor == NULL){
		monitor = "monitor_sites.list";
	}
	
	printf("LOG file: %s\n", logfile);
	printf("CSV file: %s\n", csvfile);
	printf("Monitor sites: %s\n", monitor);
//	printf("Lines: %d\n", count_line(logfile));
	
	parse_log(fp, csvfile, monitor);

	printf("DONE!\n");
	return 0;
}

int
count_lines (FILE *fp)
{
	printf ("Counting lines...\n");
	int i = 0;
	int sz;
	char *buff;
	buff = NULL;
	while(getline(&buff, &sz, fp) > 0) {
		buff = NULL;
		++i;
	}
	printf("Lines: %d\n", i);
	return i;	
}

char **
getsites(char* filename)
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char** result;
	int i = 0;
	result = malloc(N_MONITOR_SITES * sizeof(char*));
	memset(result, 0, N_MONITOR_SITES * sizeof(char*));

	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Can`t open list of sites for monitoring\n");
		exit(1);
	}

	while ((read = getline(&line, &len, fp)) != -1) {
		line[strlen(line) - 1] = '\0';
		result[i++] = line;
		line = NULL;
	}

	result[i] = "other";	
	result[i + 1] = NULL;

	fclose(fp);

	return result;
}

int
is_exist_elem (char *elem, char **array, int elem_count)
{
	int i;
	for (i=0; i < elem_count; i++) {
		if (strcmp(array[i], elem) == 0)
			return TRUE;
	}
	return FALSE;
}

void
progress(int global, int curent) 
{
	int i;
	float j;
	float d = 50;
//	printf("\033[1A");
	float proc;
	float g = (float) global;
	float c = (float) curent;
	proc = c/g;
	j = proc * d ;
	printf("\r[");
	for (i = 0; i <= d; ++i) {
		if (i < j)
			printf("#");
		else
			printf(".");
	}
	printf("]\t");
	printf("\t%3.2f%%", proc*100);
}

void
parse_log(FILE *fp, char *csvfile, char *monitor)
{
	int lines_c;
	if (is_verbose) {
		lines_c = count_lines(fp);
		rewind(fp);
	}
	printf("Start parse log...\n");
	user_table_t *table;
	struct log_entry entry;
	char *url;
	char *other_url;
	other_url = "other";
	int count_sites = 0;
	char **sites = getsites(monitor);
	char *tcp_miss = "TCP_MISS";
	char *get = "GET";
	int lines;

	for (count_sites = 0; count_sites <= N_MONITOR_SITES; ++count_sites) {
		if (sites[count_sites] == NULL)
			break;
	}
	
	table = user_table_new();
	lines = 0;

	while (read_record(fp, &entry) == 0) {
		lines++;

		entry.head_st[8] = '\0';

		printf("%s %s", entry.head_st, entry.method);
		if (strcmp(entry.head_st, tcp_miss) == 0 && strcmp(entry.method, get) == 0)
			printf("\tOK");
		printf("\n");
	
		url = chop_lvl2_domain(cut_site(entry.uri));
		if (!is_exist_elem(url, sites, count_sites))
			url = other_url;
		user_table_add_entry(table,
		    chop_uname(entry.username),
		    url);

		if (is_verbose && lines % N_LINES == 0) {
			progress(lines_c, lines);
		}
	}
	if (is_verbose)
		printf("\n");

//	user_table_list(table);

	user_table_write_csv(table, sites, csvfile);	

	free (sites);

	user_table_del(&table);
}

int
read_record(FILE *fp, struct log_entry *entry)
{
	int code;
	int ret;
	int seconds;

/*	ret = fscanf(fp, "%d.%d %d"
	    "%" TO_STR(IP_MAXLEN) "s "
	    "%" TO_STR(HEAD_ST_MAXLEN) "s "
	    "%d "
	    "%" TO_STR(METHOD_MAXLEN) "s "
	    "%" TO_STR(URI_MAXLEN) "s "
	    "%" TO_STR(USERNAME_MAXLEN) "s ",
	    &entry->time, &seconds, &entry->elaps,
	    entry->ipaddr, entry->head_st, &entry->len,
	    entry->method, entry->uri, entry->username
	    );*/

	ret = fscanf(fp, "%d.%d | %d | "
            "%" TO_STR(IP_MAXLEN) "s | "
            "%" TO_STR(HEAD_ST_MAXLEN) "s | "
            "%d | "
            "%" TO_STR(METHOD_MAXLEN) "s | "
            "%" TO_STR(URI_MAXLEN) "s | "
            "%" TO_STR(USERNAME_MAXLEN) "s | "
            "%" TO_STR(SH_MAXLEN) "s | "
            "%" TO_STR(TYPE_MAXLEN) "s | "
            "%d | "
            "%" TO_STR(USERAGENT_MAXLEN) "s | "
            "%" TO_STR(REFERER_MAXLEN) "s"
            &entry->time, &seconds, &entry->elaps,
            entry->ipaddr, entry->head_st, &entry->len,
            entry->method, entry->uri, entry->username,
	    entry->sh, entry->type, entry->port, entry->useragent,
	    entry->referer);

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

//FIXME maybe need to append me
//func should have bug when uri contains IPv6 address
char *
chop_domain(char *uri)
{
	int d_s, d_e;
	char *p;

	d_s = d_e = 0;

	if (strncmp(uri, "http://", 7) == 0)
		d_s = 7;
	else if (strncmp(uri, "https://", 8) == 0)
		d_s = 8;
	
	p = strchr(uri + d_s, '/');
	if (p == NULL)
		d_e = strlen(uri);
	else
		d_e = p - uri;

	//maybe we find port?
	p = strchr(uri + d_s, ':');
	if (p != NULL)
		d_e = MIN(d_e, p - uri);

	memmove(uri, uri + d_s, d_e - d_s);
	uri[d_e - d_s] = 0;

	return uri;
}

/*
 * NOTE: we assume that domname is domain name without
 * protocol name or uri path
 */
char *
chop_lvl2_domain(char *domname)
{
	int i;
	int len, n;
	unsigned char unusedbuf[sizeof(struct in6_addr)];

	//then we have ipaddr
	if (inet_pton(AF_INET, domname, (void *)unusedbuf) > 0 ||
	    inet_pton(AF_INET6, domname, (void *)unusedbuf) > 0)
		return domname;

	n = 0;
	len = strlen(domname);
	for (i = len; i >= 0; i--) {
		if (domname[i] == '.') {
			n++;
			if (n == 2)
				break;
		}
	}

	if (i > 0)
		memmove(domname, domname + i + 1, len - i - 1);

	return domname;
}

//Я написал такую функцию по отчленению домена
//Мне нужно только домены 2-го уровня, т.е. не sdfsdf.vk.com, а vk.com
char*
cut_site(char* site)
{
//        char* result;
        int i;
        int point_count = 2;
        for (i = 8; i < strlen(site); ++i) {
                if (site[i] == '/' || site[i] == ':')
                        site[i] = '\0';
        }
        for (i = strlen(site); i >= 0; --i) {
                if (site[i] == '.') {
                        if(!(isdigit(site[i + 1]) && point_count == 2))
                                --point_count;
                }
                if (site[i] == '/' || point_count == 0) {
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
