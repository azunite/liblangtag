/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * lt-database.h
 * Copyright (C) 2011-2015 Akira TAGOH
 * 
 * Authors:
 *   Akira TAGOH  <akira@tagoh.org>
 * 
 * You may distribute under the terms of either the GNU
 * Lesser General Public License or the Mozilla Public
 * License, as specified in the README file.
 */
#if !defined (__LANGTAG_H__INSIDE) && !defined (__LANGTAG_COMPILATION)
#error "Only <liblangtag/langtag.h> can be included directly."
#endif

#ifndef __LT_DATABASE_H__
#define __LT_DATABASE_H__

#include <liblangtag/lt-macros.h>
#include <liblangtag/lt-extlang-db.h>
#include <liblangtag/lt-grandfathered-db.h>
#include <liblangtag/lt-lang-db.h>
#include <liblangtag/lt-redundant-db.h>
#include <liblangtag/lt-region-db.h>
#include <liblangtag/lt-script-db.h>
#include <liblangtag/lt-variant-db.h>
#include <liblangtag/lt-relation-db.h>

LT_BEGIN_DECLS

typedef struct _lt_db_val_t	lt_db_val_t;

/**
 * lt_db_val_t:
 * @lang: The placeholder to store the object of #lt_lang_db_t.
 * @extlang: The placeholder to store the object of #lt_extlang_db_t.
 * @script: The placeholder to sthre the object of #lt_script_db_t.
 * @region: The placeholder to store the object of #lt_region_db_t.
 * @variant: The placeholder to store the object of #lt_variant_db_t.
 * @grandfathered: The placeholder to store the object of #lt_grandfathered_db_t.
 * @redundant: The placeholder to store the object of #lt_redundant_db_t.
 * @relation: The placeholder to store the object of #lt_relation_db_t.
 *
 * The database set of structure to peek instances of them. this is useful
 * for the debugging purpose only.
 */
struct _lt_db_val_t {
	lt_lang_db_t          *lang;
	lt_extlang_db_t       *extlang;
	lt_script_db_t        *script;
	lt_region_db_t        *region;
	lt_variant_db_t       *variant;
	lt_grandfathered_db_t *grandfathered;
	lt_redundant_db_t     *redundant;
	lt_relation_db_t      *relation;
};

void                   lt_db_set_datadir      (const char *path);
const char            *lt_db_get_datadir      (void);
void                   lt_db_set_val          (lt_db_val_t *val);
void                   lt_db_initialize       (void);
void                   lt_db_finalize         (void);
lt_lang_db_t          *lt_db_get_lang         (void);
lt_extlang_db_t       *lt_db_get_extlang      (void);
lt_script_db_t        *lt_db_get_script       (void);
lt_region_db_t        *lt_db_get_region       (void);
lt_variant_db_t       *lt_db_get_variant      (void);
lt_grandfathered_db_t *lt_db_get_grandfathered(void);
lt_redundant_db_t     *lt_db_get_redundant    (void);
lt_relation_db_t      *lt_db_get_relation     (void);

LT_END_DECLS

#endif /* __LT_DATABASE_H__ */
