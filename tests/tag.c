/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * tag.c
 * Copyright (C) 2011-2015 Akira TAGOH
 * 
 * Authors:
 *   Akira TAGOH  <akira@tagoh.org>
 * 
 * You may distribute under the terms of either the GNU
 * Lesser General Public License or the Mozilla Public
 * License, as specified in the README file.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "langtag.h"
#include "lt-utils.h"

int
main(int    argc,
     char **argv)
{
	lt_tag_t *tag;

	setlocale(LC_ALL, "");

	lt_db_set_datadir(TEST_DATADIR);
	lt_db_initialize();
	tag = lt_tag_new();
	if (lt_strcmp0(argv[1], "help") == 0) {
	  help:
		printf("Usage: %s <command> ...\n"
		       "commands: canonicalize, ecanonicalize, dump, from_locale, from_locale_s, lookup, match, script, to_locale, transform\n",
		       argv[0]);
	} else if (lt_strcmp0(argv[1], "canonicalize") == 0) {
		char *s;

		if (lt_tag_parse(tag, argv[2], NULL)) {
			s = lt_tag_canonicalize(tag, NULL);
			printf("%s -> %s\n", argv[2], s);
			free(s);
		}
	} else if (lt_strcmp0(argv[1], "ecanonicalize") == 0) {
		char *s;

		if (lt_tag_parse(tag, argv[2], NULL)) {
			s = lt_tag_canonicalize_in_extlang_form(tag, NULL);
			printf("%s -> %s\n", argv[2], s);
			free(s);
		}
	} else if (lt_strcmp0(argv[1], "dump") == 0) {
		if (lt_tag_parse(tag, argv[2], NULL))
			lt_tag_dump(tag);
	} else if (lt_strcmp0(argv[1], "from_locale_s") == 0) {
		lt_tag_unref(tag);
		tag = lt_tag_convert_from_locale_string(argv[2], NULL);
		if (tag) {
			const char *s = lt_tag_get_string(tag);

			printf("Tag: %s\n", s);
			lt_tag_dump(tag);
		}
	} else if (lt_strcmp0(argv[1], "from_locale") == 0) {
		lt_tag_unref(tag);
		tag = lt_tag_convert_from_locale(NULL);
		if (tag) {
			const char *s = lt_tag_get_string(tag);

			printf("Tag: %s\n", s);
			lt_tag_dump(tag);
		}
	} else if (lt_strcmp0(argv[1], "match") == 0) {
		if (lt_tag_parse(tag, argv[2], NULL)) {
			if (lt_tag_match(tag, argv[3], NULL))
				printf("%s matches with %s\n", argv[3], argv[2]);
			else
				printf("%s doesn't match with %s\n", argv[3], argv[2]);
		}
	} else if (lt_strcmp0(argv[1], "lookup") == 0) {
		if (lt_tag_parse(tag, argv[2], NULL)) {
			char *result = lt_tag_lookup(tag, argv[3], NULL);
			if (result)
				printf("%s\n", result);
			else
				printf("%s doesn't match with %s\n", argv[3], argv[2]);
			free(result);
		}
	} else if (lt_strcmp0(argv[1], "to_locale") == 0) {
		char *l;

		if (lt_tag_parse(tag, argv[2], NULL)) {
			l = lt_tag_convert_to_locale(tag, NULL);
			printf("%s -> %s\n", argv[2], l);
			free(l);
		}
	} else if (lt_strcmp0(argv[1], "transform") == 0) {
		lt_tag_t *r;

		if (lt_tag_parse(tag, argv[2], NULL)) {
			r = lt_tag_transform(tag, NULL);
			printf("%s -> %s\n", argv[2], lt_tag_get_string(r));
			lt_tag_unref(r);
		}
	} else if (lt_strcmp0(argv[1], "script") == 0) {
		if (lt_tag_parse(tag, argv[2], NULL)) {
			lt_relation_db_t *db = lt_db_get_relation();
			const lt_lang_t *lang = lt_tag_get_language(tag);
			lt_list_t *l, *ll = lt_relation_db_lookup_script_from_lang(db, lang);

			printf("%s: ", lt_lang_get_tag(lang));
			for (l = ll; l; l = lt_list_next(l)) {
				lt_script_t *script = lt_list_value(l);

				printf("%s ", lt_script_get_tag(script));
			}
			printf("\n");
			lt_list_free(ll);
			lt_relation_db_unref(db);
		}
	} else {
		goto help;
	}
	if (tag)
		lt_tag_unref(tag);
	lt_db_finalize();

	return 0;
}
