#include <stdio.h>
#include <string.h>

#include "common.h"
#include "hash.h"
#include "log.h"
#include "macros.h"
#include "users_request.h"
extern long time_h, time_l;
static struct user_item *user_item_new(char *uname);
static void user_item_add_site(struct user_item *item, char *site);
static void user_item_del(struct user_item **item);

static struct site_item *site_item_new(char *site);
static void site_item_del(struct site_item **item);

struct user_item {
	char uname[USERNAME_MAXLEN];
	//char orgname[ORGNAME_MAXLEN];
	struct hash_table *site_requests;
};

struct site_item {
	char site[SITE_MAXLEN];
	int n;
};

/*
 * Usertable funcs
 */
static unsigned long
usertable_hash_cb(const void *data)
{
        int i, mult, res;
        char *s; 
    
        mult = 31; 
        res = 0;
        s = ((struct user_item *)data)->uname;

        for (i = 0; i < strlen(s); i++)
                res = res * mult + s[i];
        return res;
}

static int
usertable_hash_compare(const void *a, const void *b)
{
	const struct user_item *pa, *pb;
	pa = a;
	pb = b;

        return strcmp(pa->uname, pb->uname);
}

user_table_t *
user_table_new()
{
	struct hash_table *table;

	table = hash_table_new(10, usertable_hash_cb, usertable_hash_compare);
	if (table == NULL)
		error(1, "table initiation fail\n");

	return table;
}

void
user_table_del(user_table_t **table)
{
	void *key, *data;
	struct hash_table_iter *iter;

	iter = hash_table_iterate_init(*table);
	
	while (hash_table_iterate(iter, &key, &data) != FALSE)
		user_item_del((struct user_item **)(&data));

	hash_table_iterate_deinit(&iter);

	hash_table_destroy(table);
}

void
user_table_add_entry(user_table_t *table, char *uname, char *site)
{
	struct user_item *tmp;

	if (hash_table_lookup(table, uname, (void **)(&tmp)) == 
			ret_not_found) {
		tmp = user_item_new(uname);
		hash_table_insert(table, tmp->uname, tmp);
	}

	user_item_add_site(tmp, site);
}

void
user_table_print(user_table_t *table, char **sites)
{
        void *key, *data;
        struct hash_table_iter *iter;
	int site_count;
	int i = 0;
	int line_s[100];	

        iter = hash_table_iterate_init(table);
	printf("Time period;%ld;%ld\n", time_l, time_h);
	printf("user;");
	while (sites[i] != NULL) {
		printf("%s;", sites[i]);
		line_s[i] = 0;
		++i;
	}
	site_count = i;

	printf("\n");
	
        while (hash_table_iterate(iter, &key, &data) != FALSE) {
                void *key2, *data2;
                struct hash_table_iter *iter2;
                struct user_item *item = data;

                iter2 = hash_table_iterate_init(item->site_requests);
                printf("%s;", item->uname);
        	while (hash_table_iterate(iter2, &key2, &data2) != FALSE) {
			i = 0;
			while (sites[i] != NULL) {
	                	struct site_item *item2 = data2;
				if (strcmp(sites[i], item2->site) == 0) {
					line_s[i] = item2->n;
					break;
				}
				++i;
			}
		}

                for (i = 0; i < site_count ; i++){
			printf("%d;", line_s[i]);
                        line_s[i] = 0;
                }

		printf("\n");
                hash_table_iterate_deinit(&iter2);
        }
	
        hash_table_iterate_deinit(&iter);
}

void
user_table_write_csv(user_table_t *table, char **sites, char *csvfile)
{
	printf("Creating CSV file...");
	FILE *fp;
        void *key, *data;
        struct hash_table_iter *iter;
	int site_count;
	int i = 0;
	int line_s[100];	

        fp = fopen(csvfile, "w");
        if (fp == NULL) {
                printf("Can`t write csv file\n");
                exit(1);
        }

        iter = hash_table_iterate_init(table);

	fprintf(fp, "Time period;%ld;%ld\n", time_l, time_h);

	fprintf(fp, "user;");
	while (sites[i] != NULL) {
		fprintf(fp, "%s;", sites[i]);
		line_s[i] = 0;
		++i;
	}
	site_count = i;

	fprintf(fp, "\n");
	
        while (hash_table_iterate(iter, &key, &data) != FALSE) {
                void *key2, *data2;
                struct hash_table_iter *iter2;
                struct user_item *item = data;

                iter2 = hash_table_iterate_init(item->site_requests);
                fprintf(fp, "%s;", item->uname);
        	while (hash_table_iterate(iter2, &key2, &data2) != FALSE) {
			i = 0;
			while (sites[i] != NULL) {
	                	struct site_item *item2 = data2;
				if (strcmp(sites[i], item2->site) == 0) {
					line_s[i] = item2->n;
					break;
				}
				++i;
			}
		}

                for (i = 0; i < site_count ; i++){
			fprintf(fp,"%d;", line_s[i]);
                        line_s[i] = 0;
                }

		fprintf(fp, "\n");
                hash_table_iterate_deinit(&iter2);
        }
	
        hash_table_iterate_deinit(&iter);
	fclose(fp);
	printf("\tdone!\n");
}

/*
 * User Item:
 */
static unsigned long
site_item_hash_cb(const void *data)
{
        int i, mult, res;
        char *s; 
    
        mult = 31; 
        res = 0;
        s = ((struct site_item *)data)->site;

        for (i = 0; i < strlen(s); i++)
                res = res * mult + s[i];
        return res;
}

static int
site_item_hash_compare(const void *a, const void *b)
{
	const struct site_item *pa, *pb;
	pa = a;
	pb = b;

        return strcmp(pa->site, pb->site);
}

struct user_item *
user_item_new(char *uname)
{
	struct user_item *item;

	item = xmalloc(sizeof(*item));

	strncpy(item->uname, uname, USERNAME_MAXLEN);

	item->site_requests = hash_table_new(10,
		       site_item_hash_cb,
		       site_item_hash_compare);
	if (item->site_requests == NULL)
		error(1, "table initiation fail\n");
	
	return item;
}

static void
user_item_del(struct user_item **item)
{
	void *key, *data;
	struct hash_table_iter *iter;
	struct hash_table *table;

	table = (*item)->site_requests;

	iter = hash_table_iterate_init(table);
	
	while (hash_table_iterate(iter, &key, (void **)(&data)) != FALSE)
		site_item_del((struct site_item**)(&data));

	hash_table_iterate_deinit(&iter);

	hash_table_destroy(&table);

	free(*item);
	*item = NULL;
}

static void
user_item_add_site(struct user_item *item, char *site)
{
	struct site_item *tmp;

	if (hash_table_lookup(item->site_requests, site, (void **)(&tmp)) == ret_not_found) {
		tmp = site_item_new(site);
		hash_table_insert(item->site_requests, tmp->site, tmp);
	}

	tmp->n++;
}

/*
 *
 */
static struct site_item *
site_item_new(char *site)
{
	struct site_item *item;

	item = malloc(sizeof(*item));
	strncpy(item->site, site, SITE_MAXLEN);
	item->n = 0;

	return item;
}

static void
site_item_del(struct site_item **item)
{
	free(*item);
	*item = NULL;
}
