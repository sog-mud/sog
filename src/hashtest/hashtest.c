/* $Id: hashtest.c,v 1.6 2001-09-12 12:32:16 fjoe Exp $ */
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#include <typedef.h>
#include <str.h>
#include <buffer.h>
#include <varr.h>
#include <hash.h>
#include <strkey_hash.h>

varr v;
hash_t h;

int nelem;
int niter;

void *
strkey_cpy(void *p, const void *q)
{
	return (void *) *(const char **) p = *(const char **) q;
}

static hashdata_t h_data =
{
	sizeof(const char *), 1,
	NULL,
	NULL,
	strkey_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_str
};

static varrdata_t v_data =
{
	sizeof(const char *), 4
};

void search_varr(const char *);
void bsearch_varr(const char *);
void search_hash(const char *);

int
main(int argc, char *argv[])
{
	const char *text;

	int i;
	char **pp;
	char *p;

	if (argc != 4) {
		printf("Syntax: %s <text> <nelem> <niter>\n", argv[0]);
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
	varr_init(&v, &v_data);
	for (i = 0; i < nelem; i++) {
		pp = varr_enew(&v);
		asprintf(pp, "%04d: %s", i, text);
	}

	/*
	 * initialize h
	 */
	hash_init(&h, &h_data);
	for (i = 0; i < nelem; i++) {
		asprintf(&p, "%04d: %s", i, text);
		hash_insert(&h, p, &p);
	}

	printf("%d elems (%d times)\n", nelem, niter);
	setbuf(stdout, NULL);

	/*
	 * search in varr
	 */
	asprintf(&p, "%04d: %s", 0, text);
	search_varr(p);

	asprintf(&p, "%04d: %s", nelem/2, text);
	search_varr(p);

	asprintf(&p, "%04d: %s", nelem-1, text);
	search_varr(p);

	/*
	 * bsearch in varr
	 */
	asprintf(&p, "%04d: %s", nelem/2, text);
	bsearch_varr(p);

	asprintf(&p, "%04d: %s", nelem*6/7, text);
	bsearch_varr(p);

	asprintf(&p, "%04d: %s", nelem-1, text);
	bsearch_varr(p);

	/*
	 * search in hash
	 */
	asprintf(&p, "%04d: %s", 0, text);
	search_hash(p);

	asprintf(&p, "%04d: %s", nelem/2, text);
	search_hash(p);

	asprintf(&p, "%04d: %s", nelem-1, text);
	search_hash(p);

	return 0;
}

void
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

void *
v_search_cb(void *p, va_list ap)
{
	const char **pp = (const char **) p;
	const char *s = va_arg(ap, const char *);

	if (!str_cmp(*pp, s))
		return pp;

	return NULL;
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
		void *pp = varr_foreach(&v, v_search_cb, p);
		if (pp == NULL) {
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
search_hash(const char *p)
{
	int i;
	struct timeval t1;
	struct timeval t2;

	printf("Hash lookup (%s): ", p);
	gettimeofday(&t1, NULL);
	for (i = 0; i < niter; i++) {
		void *pp = strkey_search(&h, p);
		if (pp == NULL) {
			printf("failed\n");
			exit(1);
		}
	}
	gettimeofday(&t2, NULL);
	print_stats(&t1, &t2);
}
