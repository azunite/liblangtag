/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * lt-relation-db.c
 * Copyright (C) 2015-2016 Akira TAGOH
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

#include <stdlib.h>
#include <string.h>
#include <libxml/xpath.h>
#include "lt-database.h"
#include "lt-error.h"
#include "lt-list.h"
#include "lt-lock.h"
#include "lt-mem.h"
#include "lt-messages.h"
#include "lt-trie.h"
#include "lt-utils.h"
#include "lt-xml.h"
#include "lt-relation-db.h"

/**
 * SECTION:lt-relation-db
 * @Short_Description: An interface to access relation database
 * @Title: Database - Relation
 *
 * This class providdes an interface to access Relation database.
 */
struct _lt_relation_db_t {
	lt_mem_t   parent;
	lt_trie_t *relation_l_s_entries;
	lt_trie_t *relation_s_l_entries;
};

LT_LOCK_DEFINE_STATIC (rdb);

/*< private >*/
static lt_bool_t
lt_relation_db_parse(lt_relation_db_t  *relationdb,
		     lt_error_t       **error)
{
	lt_bool_t retval = TRUE;
	xmlDocPtr doc = NULL;
	xmlXPathContextPtr xctxt = NULL;
	xmlXPathObjectPtr xobj = NULL;
	lt_error_t *err = NULL;
	int i, n;
	lt_lang_db_t *langdb = NULL;
	lt_script_db_t *scriptdb = NULL;
	lt_xml_t *xml;

	lt_return_val_if_fail (relationdb != NULL, FALSE);

	relationdb->relation_l_s_entries = lt_trie_new();
	lt_mem_add_ref((lt_mem_t *)relationdb, relationdb->relation_l_s_entries,
		       (lt_destroy_func_t)lt_trie_unref);
	relationdb->relation_s_l_entries = lt_trie_new();
	lt_mem_add_ref((lt_mem_t *)relationdb, relationdb->relation_s_l_entries,
		       (lt_destroy_func_t)lt_trie_unref);

	xml = lt_xml_new();
	doc = lt_xml_get_cldr(xml, LT_XML_CLDR_SUPPLEMENTAL_SUPPLEMENTAL_DATA);
	xctxt = xmlXPathNewContext(doc);
	if (!xctxt) {
		lt_error_set(&err, LT_ERR_OOM,
			     "Unable to create an instance of xmlXPathContextPtr.");
		goto bail;
	}
	xobj = xmlXPathEvalExpression((const xmlChar *)"/supplementalData/languageData/language[@scripts]", xctxt);
	if (!xobj) {
		lt_error_set(&err, LT_ERR_FAIL_ON_XML,
			     "No valid elements for %s",
			     doc->name);
		goto bail;
	}
	n = xmlXPathNodeSetGetLength(xobj->nodesetval);

	langdb = lt_db_get_lang();
	scriptdb = lt_db_get_script();
	for (i = 0; i < n; i++) {
		xmlNodePtr ent = xmlXPathNodeSetItem(xobj->nodesetval, i);
		xmlChar *type, *scripts;
		char *p, *pp;
		lt_list_t *l;
		lt_lang_t *ol;
		lt_script_t *os;
		lt_bool_t alloced;

		type = xmlGetProp(ent, (const xmlChar *)"type");
		scripts = xmlGetProp(ent, (const xmlChar *)"scripts");
		p = (char *)scripts;
		do {
			pp = strchr(p, ' ');
			if (pp) {
				*pp = 0;
				pp++;
			}
			ol = lt_lang_db_lookup(langdb, (const char *)type);
			os = lt_script_db_lookup(scriptdb, p);
			if (!ol || !os) {
				if (!ol)
					lt_warning("Unknown lang tag: %s", type);
				else
					lt_lang_unref(ol);
				if (!os)
					lt_warning("Unknown script tag: %s", p);
				else
					lt_script_unref(os);
				goto bail1;
			}
			alloced = FALSE;
			l = lt_trie_lookup(relationdb->relation_l_s_entries,
					   (const char *)type);
			if (!l)
				alloced = TRUE;
			l = lt_list_append(l, os, (lt_destroy_func_t)lt_script_unref);
			if (alloced) {
				lt_mem_add_ref((lt_mem_t *)relationdb->relation_l_s_entries,
					       l, (lt_destroy_func_t)lt_list_free);
			}
			lt_trie_replace(relationdb->relation_l_s_entries,
					(const char *)type, l, NULL);
			alloced = FALSE;
			l = lt_trie_lookup(relationdb->relation_s_l_entries, p);
			if (!l)
				alloced = TRUE;
			l = lt_list_append(l, ol, (lt_destroy_func_t)lt_lang_unref);
			if (alloced) {
				lt_mem_add_ref((lt_mem_t *)relationdb->relation_s_l_entries,
					       l, (lt_destroy_func_t)lt_list_free);
			}
			lt_trie_replace(relationdb->relation_s_l_entries, p, l, NULL);
		  bail1:
			p = pp;
		} while (p);
			
		xmlFree(type);
		xmlFree(scripts);
	}

  bail:
	if (lt_error_is_set(err, LT_ERR_ANY)) {
		if (error)
			*error = lt_error_ref(err);
		else
			lt_error_print(err, LT_ERR_ANY);
		lt_error_unref(err);
		retval = FALSE;
	}
	if (langdb)
		lt_lang_db_unref(langdb);
	if (scriptdb)
		lt_script_db_unref(scriptdb);
	if (xobj)
		xmlXPathFreeObject(xobj);
	if (xctxt)
		xmlXPathFreeContext(xctxt);
	if (xml)
		lt_xml_unref(xml);

	return retval;
}

/*< public >*/
/**
 * lt_relation_db_new:
 *
 * Create a new instance of a #lt_relation_db_t.
 *
 * Returns: (transfer full): a new instance of #lt_relation_db_t.
 */
lt_relation_db_t *
lt_relation_db_new(void)
{
	lt_relation_db_t *retval = lt_mem_alloc_object(sizeof (lt_relation_db_t));

	return retval;
}

/**
 * lt_relation_db_ref:
 * @relationdb: a #lt_relation_db_t.
 *
 * Increases the reference count of @relationdb.
 *
 * Returns: (transfer none): the same @relationddb object.
 */
lt_relation_db_t *
lt_relation_db_ref(lt_relation_db_t *relationdb)
{
	lt_return_val_if_fail (relationdb != NULL, NULL);

	return lt_mem_ref((lt_mem_t *)relationdb);
}

/**
 * lt_relation_db_unref:
 * @relationdb: a #lt_relation_db_t.
 *
 * Decreases the reference count of @relationdb. when its reference count
 * drops to 0, the object is finalized (i.e. its memory is freed).
 */
void
lt_relation_db_unref(lt_relation_db_t *relationdb)
{
	if (relationdb)
		lt_mem_unref((lt_mem_t *)relationdb);
}

/**
 * lt_relation_db_lookup_lang_from_script:
 * @relationdb: a #lt_relation_db_t.
 * @script: a #lt_script_t.
 *
 * Look up the languages corresponding to the script @script.
 *
 * Returns: (transfer full): a #lt_list_t containing #lt_lang_t object.
 */
lt_list_t *
lt_relation_db_lookup_lang_from_script(lt_relation_db_t  *relationdb,
				       const lt_script_t *script)
{
	lt_list_t *l, *ll, *retval = NULL;
	char *key;

	lt_return_val_if_fail (relationdb != NULL, NULL);
	lt_return_val_if_fail (script != NULL, NULL);

	LT_LOCK (rdb);
	if (!relationdb->relation_s_l_entries) {
		if (!lt_relation_db_parse(relationdb, NULL)) {
			LT_UNLOCK (rdb);
			return NULL;
		}
	}
	LT_UNLOCK (rdb);

	key = strdup(lt_script_get_name(script));
	l = lt_trie_lookup(relationdb->relation_s_l_entries,
			   lt_strlower(key));
	free(key);

	for (ll = l; ll; ll = lt_list_next(ll)) {
		retval = lt_list_append(retval,
					lt_lang_ref(lt_list_value(ll)),
					(lt_destroy_func_t)lt_lang_unref);
	}

	return retval;
}

/**
 * lt_relation_db_lookup_script_from_lang:
 * @relationdb: a #lt_relation_db_t.
 * @lang: a #lt_lang_t.
 *
 * Look up the scripts corresponding to the language @lang.
 *
 * Returns: (transfer full): a #lt_list_t containing #lt_script_t objects.
 */
lt_list_t *
lt_relation_db_lookup_script_from_lang(lt_relation_db_t *relationdb,
				       const lt_lang_t  *lang)
{
	lt_list_t *l, *ll, *retval = NULL;
	char *key;

	lt_return_val_if_fail (relationdb != NULL, NULL);
	lt_return_val_if_fail (lang != NULL, NULL);

	LT_LOCK (rdb);
	if (!relationdb->relation_l_s_entries) {
		if (!lt_relation_db_parse(relationdb, NULL)) {
			LT_UNLOCK (rdb);
			return NULL;
		}
	}
	LT_UNLOCK (rdb);

	key = strdup(lt_lang_get_tag(lang));
	l = lt_trie_lookup(relationdb->relation_l_s_entries,
			   lt_strlower(key));
	free(key);

	for (ll = l; ll; ll = lt_list_next(ll)) {
		retval = lt_list_append(retval,
					lt_script_ref(lt_list_value(ll)),
					(lt_destroy_func_t)lt_lang_unref);
	}

	return retval;
}
