/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * lt-tag.h
 * Copyright (C) 2011-2012 Akira TAGOH
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

#ifndef __LT_TAG_H__
#define __LT_TAG_H__

#include <liblangtag/lt-macros.h>
#include <liblangtag/lt-list.h>
#include <liblangtag/lt-string.h>
#include <liblangtag/lt-lang.h>
#include <liblangtag/lt-extlang.h>
#include <liblangtag/lt-script.h>
#include <liblangtag/lt-region.h>
#include <liblangtag/lt-extension.h>
#include <liblangtag/lt-grandfathered.h>

LT_BEGIN_DECLS

/**
 * lt_tag_t:
 *
 * All the fields in the <structname>lt_tag_t</structname>
 * structure are private to the #lt_tag_t implementation.
 */
typedef struct _lt_tag_t	lt_tag_t;

enum _lt_tag_filter_t {
	LT_TAG_FILTER_NONE          = 0,
	LT_TAG_FILTER_LANGUAGE      = 1 << 0,
	LT_TAG_FILTER_EXTLANG       = 1 << 1,
	LT_TAG_FILTER_SCRIPT        = 1 << 2,
	LT_TAG_FILTER_REGION        = 1 << 3,
	LT_TAG_FILTER_VARIANT       = 1 << 4,
	LT_TAG_FILTER_EXTENSION     = 1 << 5,
	LT_TAG_FILTER_PRIVATEUSE    = 1 << 6,
	LT_TAG_FILTER_GRANDFATHERED = 1 << 7,
	LT_TAG_FILTER_ALL           = -1
};

/**
 * lt_tag_filter_t:
 * @LT_TAG_FILTER_NONE: all of objects filtered.
 * @LT_TAG_FILTER_LANGUAGE: include #lt_lang_t object into the result.
 * @LT_TAG_FILTER_EXTLANG: include #lt_extlang_t object into the result.
 * @LT_TAG_FILTER_SCRIPT: include #lt_script_t object into the result.
 * @LT_TAG_FILTER_REGION: include #lt_region_t object into the result.
 * @LT_TAG_FILTER_VARIANT: include #lt_variant_t object into the result.
 * @LT_TAG_FILTER_EXTENSION: include #lt_extension_t object into the result.
 * @LT_TAG_FILTER_PRIVATEUSE: include the privateuse object into the result.
 * @LT_TAG_FILTER_GRANDFATHERED: include #lt_grandfathered_t object into the result.
 * @LT_TAG_FILTER_ALL: no objects filtered.
 *
 * This is used as a binary count sequence with lt_tag_get_string_with_filter()
 * to allow only given tags to be the outcome.
 */
typedef enum _lt_tag_filter_t	lt_tag_filter_t;

lt_tag_t                 *lt_tag_new                         (void);
lt_tag_t                 *lt_tag_ref                         (lt_tag_t       *tag);
void                      lt_tag_unref                       (lt_tag_t       *tag);
lt_bool_t                 lt_tag_parse                       (lt_tag_t       *tag,
                                                              const char     *tag_string,
                                                              lt_error_t     **error);
lt_bool_t                 lt_tag_parse_with_extra_token      (lt_tag_t       *tag,
                                                              const char     *tag_string,
                                                              lt_error_t     **error);
void                      lt_tag_clear                       (lt_tag_t       *tag);
lt_tag_t                 *lt_tag_copy                        (const lt_tag_t *tag);
lt_bool_t                 lt_tag_truncate                    (lt_tag_t       *tag,
                                                              lt_error_t     **error);
const char               *lt_tag_get_string_with_filter      (lt_tag_t       *tag,
							      int             filter);
const char               *lt_tag_get_string                  (lt_tag_t       *tag);
char                     *lt_tag_canonicalize                (lt_tag_t       *tag,
                                                              lt_error_t     **error);
char                     *lt_tag_canonicalize_in_extlang_form(lt_tag_t       *tag,
                                                              lt_error_t     **error);
char                     *lt_tag_convert_to_locale           (lt_tag_t       *tag,
                                                              lt_error_t     **error);
lt_tag_t                 *lt_tag_convert_from_locale         (lt_error_t     **error);
lt_tag_t                 *lt_tag_convert_from_locale_string  (const char     *locale,
                                                              lt_error_t     **error);
void                      lt_tag_dump                        (const lt_tag_t *tag);
lt_bool_t                 lt_tag_compare                     (const lt_tag_t *v1,
                                                              const lt_tag_t *v2);
lt_bool_t                 lt_tag_match                       (const lt_tag_t *v1,
                                                              const char     *v2,
                                                              lt_error_t     **error);
char                     *lt_tag_lookup                      (const lt_tag_t *tag,
                                                              const char     *pattern,
                                                              lt_error_t     **error);
char                     *lt_tag_transform                   (lt_tag_t       *tag,
                                                              lt_error_t     **error);
const lt_lang_t          *lt_tag_get_language                (const lt_tag_t *tag);
const lt_extlang_t       *lt_tag_get_extlang                 (const lt_tag_t *tag);
const lt_script_t        *lt_tag_get_script                  (const lt_tag_t *tag);
const lt_region_t        *lt_tag_get_region                  (const lt_tag_t *tag);
const lt_list_t          *lt_tag_get_variants                (const lt_tag_t *tag);
const lt_extension_t     *lt_tag_get_extension               (const lt_tag_t *tag);
const lt_string_t        *lt_tag_get_privateuse              (const lt_tag_t *tag);
const lt_grandfathered_t *lt_tag_get_grandfathered           (const lt_tag_t *tag);

LT_END_DECLS

#endif /* __LT_TAG_H__ */
