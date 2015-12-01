/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * lt-relation-db.h
 * Copyright (C) 2015 Akira TAGOH
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

#ifndef __LT_RELATION_DB_H__
#define __LT_RELATION_DB_H__

#include <liblangtag/lt-macros.h>

LT_BEGIN_DECLS

/**
 * lt_relation_db_t:
 *
 * All the fields in the <structname>lt_relation_db_t</structname>
 * structure are private to the #lt_relation_db_t implementation.
 */
typedef struct _lt_relation_db_t	lt_relation_db_t;


lt_relation_db_t *lt_relation_db_new                    (void);
lt_relation_db_t *lt_relation_db_ref                    (lt_relation_db_t  *relationdb);
void              lt_relation_db_unref                  (lt_relation_db_t  *relationdb);
lt_list_t        *lt_relation_db_lookup_lang_from_script(lt_relation_db_t  *relationdb,
                                                         const lt_script_t *script);
lt_list_t        *lt_relation_db_lookup_script_from_lang(lt_relation_db_t  *relationdb,
                                                         const lt_lang_t   *lang);

LT_END_DECLS

#endif /* __LT_RELATION_DB_H__ */
