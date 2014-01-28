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
#define HSTATUS_MAXLEN 128
#define MIMETYPE_MAXLEN 128
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
	char h_status[HSTATUS_MAXLEN];
	char mime_type[MIMETYPE_MAXLEN];
	int port;
	char user_agent[USERAGENT_MAXLEN];
	char referer[REFERER_MAXLEN];
};

void process_args(int argc, char **argv);
void parse_log(FILE *fp, user_table_t *table, char **sites, int count_sites);
void parse_input(char *csvfile, char *monitor);
void progress(int global, int curent);
char *chop_uname(char *uname);
char *chop_lvl2_domain(char *domname);
char *cut_site(char *site);
char **getsites(char* filename);
int exist_elem(char *elem, char **array, int elem_count);
int count_lines(FILE *fp);
int read_record(FILE *fp, struct log_entry *entry);

long time_h = 0;
long time_l = 9999999999;

void
get_help()
{
	printf("\nSQUID REPORT GENERATOR\n\n");
	printf("Use:\n");
	printf("-m\tmonitoring sites list file\n");
	printf("-o\toutput file for csv format\n");
	printf("-v\tverbose mod");
	printf("\n\n");
	exit(1);
}

#define N_LINES 10000
int is_verbose = 0;
char *csvfile = NULL;
char *monitor = NULL;

int
main(int argc, char* argv[])
{
	int res;
	
	while ((res = getopt(argc,argv,"l:o:m:vh")) != -1){
		switch (res) {
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
		case 'h': 
			get_help(); 
		default: 
			printf("Error found !\n");
			exit(0);
        	};
	};
	argc -= optind;
	argv += optind;

	if (monitor == NULL) {
		warning("You need specify sites for monitoring\n");
		get_help();
		exit(1);
	}
	
	if (is_verbose) {
		if (csvfile)
			printf("Output CSV file: %s\n", csvfile);
		else
			printf("Output: STDOUT\n");
		printf("Monitor sites: %s\n", monitor);
	}

	process_args(argc, argv);

	return 0;
}

void
process_args(int argc, char **argv)
{
	FILE *fp;
	char **sites;
	int n;
	user_table_t *table;
       
	sites = getsites(monitor);
	for (n = 0; n <= N_MONITOR_SITES; ++n) {
		if (sites[n] == NULL)
			break;
	}

	table = user_table_new();

	if (argc == 0) {
		parse_log(stdin, table, sites, n);
		goto end;
	}

	while (argc--) {
		if (is_verbose)
			printf ("=> Parsing logfile: %s\n", *argv);
		fp = fopen(*argv++, "r");

		if (fp == NULL) {
			DEBUG(LOG_DEFAULT, "fopen error\n");
			continue;
		}
		parse_log(fp, table, sites, n);
		fclose(fp);
	}

end:
	if (is_verbose) {
		printf("DONE!\n");
	}

//	printf("Time period;%ld;%ld\n", time_l, time_h);
	
	if (csvfile != NULL)
		user_table_write_csv(table, sites, csvfile);	
	else
		user_table_print(table, sites);

	free (sites);

	user_table_del(&table);
}

int
count_lines (FILE *fp)
{
	printf ("Counting lines...\n");
	char ch;
	int lines = 0;

	while ((ch = fgetc(fp)) != EOF) {
    		if (ch=='\n')
        		++lines;
	}

	printf("done!\tLines: %d\n", lines); 

	return lines;	
}

//FIXME: может попытаться писать за границы массив
//FIXME: не очищается память, выделенная getline,
// более того не понятно как её чистить если последний элемент массива это
// указатель на константу.
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

	for (i = 0; i < elem_count; i++) {
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
	float proc;
	float g = (float) global;
	float c = (float) curent;

	proc = c/g;
	j = proc * d;
	printf("\r[");
	for (i = 0; i <= d; ++i) {
		if (i <= j)
			printf("#");
		else
			printf(" ");
	}
	printf("]\t%.2f%%", proc * 100);
	if (proc == 1) {
		printf("\n");
	}
}

void
parse_log(FILE *fp, user_table_t *table, char **sites, int count_sites)
{
	int lines_c = 0;
	struct log_entry entry;
	char *url;
	char *other_url;
	int lines;
	char *referer;
	other_url = "other";

	if (is_verbose && fp) {
		lines_c = count_lines(fp);
		rewind(fp);
	}

	lines = 0;

	while (read_record(fp, &entry) == 0) {
		lines++;
		if (entry.time > time_h) {
			time_h = entry.time;
		}
		if (entry.time < time_l) {
			time_l = entry.time;
		}
		entry.head_st[8] = '\0';
		url = chop_lvl2_domain(cut_site(entry.uri));
		referer = chop_lvl2_domain(cut_site(entry.referer));
		if (strcmp(entry.head_st, "TCP_MISS") == 0
		    && strcmp(entry.method, "GET") == 0
		    && strcmp(entry.mime_type, "text/html") == 0
		    && (strcmp(referer, "-") == 0 || strcmp(referer, url) == 0)) {
			if (!is_exist_elem(url, sites, count_sites))
				url = other_url;
			user_table_add_entry(table,
			    chop_uname(entry.username),
			    url);
		}
		if (is_verbose && (lines % N_LINES == 0 || lines == lines_c)) {
			progress(lines_c, lines);
		}
	}
}

int
read_record(FILE *fp, struct log_entry *entry)
{
	int code;
	int ret;
	int seconds;
	ret = fscanf(fp, 
		"%d.%d | %d | "
		"%" TO_STR(IP_MAXLEN) "s | "
		"%" TO_STR(HEAD_ST_MAXLEN) "s | "
		"%d | "
		"%" TO_STR(METHOD_MAXLEN) "s | "
		"%" TO_STR(URI_MAXLEN) "s | "
		"%" TO_STR(USERNAME_MAXLEN) "s | "
		"%" TO_STR(HSTATUS_MAXLEN) "s | "
		"%" TO_STR(MIMETYPE_MAXLEN) "s | "
		"%d | "
		"%" TO_STR(REFERER_MAXLEN) "s | "
		"%" TO_STR(USERAGENT_MAXLEN) "s",
            	&entry->time, &seconds, &entry->elaps,
		entry->ipaddr,
		entry->head_st,
		&entry->len,
		entry->method,
		entry->uri,
		entry->username,
		entry->h_status,
		entry->mime_type,
		&entry->port,
		entry->referer,
		entry->user_agent);

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

char*
cut_site(char* site)
{
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
	return site;
}
