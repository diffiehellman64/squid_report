#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include "log.h"

#define N_FIELDS 10

//black magic
#define TO_STR(arg) _TO_STR(arg)
#define _TO_STR(arg) #arg
//end of black magic

#define	TIME_MAXLEN 128
#define	IP_MAXLEN  16
#define	HEAD_ST_MAXLEN 128
#define	METHOD_MAXLEN 128
#define	URI_MAXLEN 8192
#define	USERNAME_MAXLEN 128

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


void
parse_log(FILE *fp)
{
	struct log_entry entry;

	while (read_record(fp, &entry) == 0) {
		printf("user %s uri %s\n", 
		    chop_uname(entry.username),
		    chop_domain(entry.uri));
	}
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

