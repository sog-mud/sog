/*
 * $Id: hashtest.c,v 1.9 2004-06-28 19:20:58 tatyana Exp $
 */

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#include <typedef.h>
#include <str.h>
#include <buffer.h>
#include <varr.h>
#include <avltree.h>
#include <container.h>
#include <memalloc.h>

varr v;
avltree_t a;

int nelem;
int niter;

static avltree_info_t a_data =
{
	&avltree_ops,
	strkey_init, strkey_destroy,
	MT_PVOID, sizeof(char *), ke_cmp_str
};

static varr_info_t v_data =
{
	&varr_ops,
	strkey_init, strkey_destroy,
	sizeof(const char *), 4
};

void search_varr(const char *);
void bsearch_varr(const char *);
void search_avltree(const char *);

int
main(int argc, char *argv[])
{
	const char *text;

	int i;
	const char **pp;
	const char *p;

	if (argc != 4) {
		printf("Usage: %s <text> <nelem> <niter>\n", argv[0]);
		return 1;
	}

	text = argv[1];

	nelem = atoi(argv[2]);
	if (nelem <= 0) {
		printf("<nelem> should be numeric value > 0\n");
		return 1;
	}

	niter = atoi(argv[3]);
	if (niter <= 0) {
		printf("<niter> should be numeric value > 0\n");
		return 1;
	}

	/*
	 * initialize v
	 */
	c_init(&v, &v_data);
	for (i = 0; i < nelem; i++) {
		pp = varr_enew(&v);
		*pp = str_printf("%04d: %s", i, text);
	}

	/*
	 * initialize h
	 */
	c_init(&a, &a_data);
	for (i = 0; i < nelem; i++) {
		p = str_printf("%04d: %s", i, text);
		pp = c_insert(&a, p);
		*pp = p;
	}

	printf("%d elems (%d times)\n", nelem, niter);
	setbuf(stdout, NULL);

	/*
	 * search in varr
	 */
	p = str_printf("%04d: %s", 0, text);
	search_varr(p);
	free_string(p);

	p = str_printf("%04d: %s", nelem/2, text);
	search_varr(p);
	free_string(p);

	p = str_printf("%04d: %s", nelem-1, text);
	search_varr(p);
	free_string(p);

	/*
	 * bsearch in varr
	 */
	p = str_printf("%04d: %s", nelem/2, text);
	bsearch_varr(p);
	free_string(p);

	p = str_printf("%04d: %s", nelem*6/7, text);
	bsearch_varr(p);
	free_string(p);

	p = str_printf("%04d: %s", nelem-1, text);
	bsearch_varr(p);
	free_string(p);

	/*
	 * search in hash
	 */
	p = str_printf("%04d: %s", 0, text);
	search_avltree(p);
	free_string(p);

	p = str_printf("%04d: %s", nelem/2, text);
	search_avltree(p);
	free_string(p);

	p = str_printf("%04d: %s", nelem-1, text);
	search_avltree(p);
	free_string(p);

	return 0;
}

static void
print_stats(struct timeval *t1, struct timeval *t2)
{
	long sec_delta = t2->tv_sec - t1->tv_sec;
	long usec_delta = t2->tv_usec - t1->tv_usec;

	if (usec_delta < 0) {
		sec_delta--;
		usec_delta += 1000000;
	}

	printf("%ld.%06ld sec\n", sec_delta, usec_delta);
}

void
search_varr(const char *p)
{
	int i;
	struct timeval t1;
	struct timeval t2;

	printf("Linear search in varr (%s): ", p);
	gettimeofday(&t1, NULL);
	for (i = 0; i < niter; i++) {
		const char **pp;
		bool found = FALSE;

		C_FOREACH (const char *, pp, &v) {
			if (!strcmp(*pp, p)) {
				found = TRUE;
				break;
			}
		}
		if (!found) {
			printf("failed\n");
			exit(1);
		}
	}
	gettimeofday(&t2, NULL);
	print_stats(&t1, &t2);
}

void
bsearch_varr(const char *p)
{
	int i;
	struct timeval t1;
	struct timeval t2;

	printf("Binary search in varr (%s): ", p);
	gettimeofday(&t1, NULL);
	for (i = 0; i < niter; i++) {
		void *pp = varr_bsearch(&v, &p, cmpstr);
		if (pp == NULL) {
			printf("failed\n");
			exit(1);
		}
	}
	gettimeofday(&t2, NULL);
	print_stats(&t1, &t2);
}

void
search_avltree(const char *p)
{
	int i;
	struct timeval t1;
	struct timeval t2;

	printf("AVL tree lookup (%s): ", p);
	gettimeofday(&t1, NULL);
	for (i = 0; i < niter; i++) {
		void *pp = c_strkey_search(&a, p);
		if (pp == NULL) {
			printf("failed\n");
			exit(1);
		}
	}
	gettimeofday(&t2, NULL);
	print_stats(&t1, &t2);
}
