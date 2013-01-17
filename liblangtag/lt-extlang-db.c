/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * lt-extlang-db.c
 * Copyright (C) 2011-2012 Akira TAGOH
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

#include <string.h>
#include <libxml/xpath.h>
#include "lt-error.h"
#include "lt-extlang.h"
#include "lt-extlang-private.h"
#include "lt-iter-private.h"
#include "lt-mem.h"
#include "lt-messages.h"
#include "lt-trie.h"
#include "lt-utils.h"
#include "lt-xml.h"
#include "lt-extlang-db.h"


/**
 * SECTION:lt-extlang-db
 * @Short_Description: An interface to access Extlang Database
 * @Title: Database - Extlang
 *
 * This class provides an interface to access Extlang database. which has been
 * registered as ISO 639 code.
 */
struct _lt_extlang_db_t {
	lt_iter_tmpl_t  parent;
	lt_xml_t       *xml;
	lt_trie_t      *extlang_entries;
};
typedef struct _lt_extlang_db_iter_t {
	lt_iter_t  parent;
	lt_iter_t *iter;
} lt_extlang_db_iter_t;

/*< private >*/
static lt_iter_t *
_lt_extlang_db_iter_init(lt_iter_tmpl_t *tmpl)
{
	lt_extlang_db_iter_t *retval;
	lt_extlang_db_t *extlangdb = (lt_extlang_db_t *)tmpl;

	retval = malloc(sizeof (lt_extlang_db_iter_t));
	if (retval) {
		retval->iter = LT_ITER_INIT (extlangdb->extlang_entries);
		if (!retval->iter) {
			free(retval);
			retval = NULL;
		}
	}

	return &retval->parent;
}

static void
_lt_extlang_db_iter_fini(lt_iter_t *iter)
{
	lt_extlang_db_iter_t *db_iter = (lt_extlang_db_iter_t *)iter;

	lt_iter_finish(db_iter->iter);
}

static lt_bool_t
_lt_extlang_db_iter_next(lt_iter_t    *iter,
			 lt_pointer_t *key,
			 lt_pointer_t *val)
{
	lt_extlang_db_iter_t *db_iter = (lt_extlang_db_iter_t *)iter;

	return lt_iter_next(db_iter->iter, key, val);
}

/*< public >*/
/**
 * lt_extlang_db_new:
 *
 * Create a new instance of a #lt_extlang_db_t.
 *
 * Returns: (transfer full): a new instance of #lt_extlang_db_t.
 */
lt_extlang_db_t *
lt_extlang_db_new(void)
{
	lt_extlang_db_t *retval = lt_mem_alloc_object(sizeof (lt_extlang_db_t));

	if (retval) {
		lt_error_t *err = NULL;
		lt_extlang_t *le;

		LT_ITER_TMPL_INIT (&retval->parent, _lt_extlang_db);

		retval->extlang_entries = lt_trie_new();
		lt_mem_add_ref((lt_mem_t *)retval, retval->extlang_entries,
			       (lt_destroy_func_t)lt_trie_unref);

		le = lt_extlang_create();
		lt_extlang_set_tag(le, "*");
		lt_extlang_set_name(le, "Wildcard entry");
		lt_trie_replace(retval->extlang_entries,
				lt_extlang_get_tag(le),
				le,
				(lt_destroy_func_t)lt_extlang_unref);
		le = lt_extlang_create();
		lt_extlang_set_tag(le, "");
		lt_extlang_set_name(le, "Empty entry");
		lt_trie_replace(retval->extlang_entries,
				lt_extlang_get_tag(le),
				le,
				(lt_destroy_func_t)lt_extlang_unref);

		retval->xml = lt_xml_new();
		if (!retval->xml) {
			lt_extlang_db_unref(retval);
			retval = NULL;
			goto bail;
		}
		lt_mem_add_ref((lt_mem_t *)retval, retval->xml,
			       (lt_destroy_func_t)lt_xml_unref);
//		lt_extlang_db_parse(retval, &err);
		if (err) {
			lt_error_print(err, LT_ERR_ANY);
			lt_extlang_db_unref(retval);
			retval = NULL;
			lt_error_unref(err);
		}
	}
  bail:

	return retval;
}

/**
 * lt_extlang_db_ref:
 * @extlangdb: a #lt_extlang_db_t.
 *
 * Increases the reference count of @extlangdb.
 *
 * Returns: (transfer none): the same @extlangdb object.
 */
lt_extlang_db_t *
lt_extlang_db_ref(lt_extlang_db_t *extlangdb)
{
	lt_return_val_if_fail (extlangdb != NULL, NULL);

	return lt_mem_ref((lt_mem_t *)extlangdb);
}

/**
 * lt_extlang_db_unref:
 * @extlangdb: a #lt_extlang_db_t.
 *
 * Decreases the reference count of @extlangdb. when its reference count
 * drops to 0, the object is finalized (i.e. its memory is freed).
 */
void
lt_extlang_db_unref(lt_extlang_db_t *extlangdb)
{
	if (extlangdb)
		lt_mem_unref((lt_mem_t *)extlangdb);
}

/**
 * lt_extlang_db_lookup:
 * @extlangdb: a #lt_extlang_db_t.
 * @subtag: a subtag name to lookup.
 *
 * Lookup @lt_extlang_t if @subtag is valid and registered into the database.
 *
 * Returns: (transfer full): a #lt_extlang_t that meets with @subtag.
 *                           otherwise %NULL.
 */
lt_extlang_t *
lt_extlang_db_lookup(lt_extlang_db_t *extlangdb,
		     const char      *subtag)
{
	lt_extlang_t *retval;
	char *s;

	lt_return_val_if_fail (extlangdb != NULL, NULL);
	lt_return_val_if_fail (subtag != NULL, NULL);

	s = strdup(subtag);
	retval = lt_trie_lookup(extlangdb->extlang_entries,
				lt_strlower(s));
	free(s);
	if (retval)
		return lt_extlang_ref(retval);

	return NULL;
}
