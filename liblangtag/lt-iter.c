/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * lt-iter.c
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

#include <stdlib.h>
#include "lt-messages.h"
#include "lt-iter-private.h"

#define MAGIC_CODE	0xB1C023FF


/**
 * SECTION: lt-iter
 * @Short_Description: An interface to iterate over the key/value pairs
 *                     in an initiated object.
 * @Title: Iterator
 *
 * This class provides a common interface to iterate over the key/value pairs
 * in an initiated object.
 */
/*< private >*/

/*< protected >*/
void
lt_iter_tmpl_init(lt_iter_tmpl_t *tmpl)
{
	lt_return_if_fail (tmpl != NULL);

	tmpl->magic_code = MAGIC_CODE;
}

/*< public >*/
/**
 * lt_iter_ref:
 * @iter: a #lt_iter_t
 *
 * Increases the reference count of @iter.
 *
 * Returns: (transfer none): the same @iter object.
 */
lt_iter_t *
lt_iter_ref(lt_iter_t *iter)
{
	lt_iter_tmpl_t *tmpl;

	lt_return_val_if_fail (iter != NULL, NULL);

	tmpl = iter->target;

	return lt_mem_ref(&tmpl->parent);
}

/**
 * lt_iter_unref:
 * @iter: a #lt_iter_t
 *
 * Decreases the reference count of @iter. when its reference count
 * drops to 0, the object is finalized (i.e. its memory is freed).
 */
void
lt_iter_unref(lt_iter_t *iter)
{
	lt_return_if_fail (iter != NULL);

	lt_mem_unref(&iter->target->parent);
}

/**
 * lt_iter_init:
 * @tmpl: a #lt_iter_tmpl_t
 *
 * Initialize the iterator with @tmpl object. this function has to be called
 * before performing any opperation with the iterator and lt_iter_finish()
 * when the iterator isn't needed anymore.
 * |[
 * lt_lang_db *lang = lt_lang_db_new();
 * lt_pointer_t *key, *val;
 * lt_iter_t *iter;
 *
 * iter = LT_ITER_INIT (lang);
 * while (lt_iter_next(iter, &key, &val)) {
 *   /&ast; do something with key and value &ast;/
 * }
 * lt_iter_finish(iter);
 * ]|
 *
 * Returns: (transfer none): the initialized iterator object.
 */
lt_iter_t *
lt_iter_init(lt_iter_tmpl_t *tmpl)
{
	lt_iter_t *retval;

	lt_return_val_if_fail (tmpl != NULL, NULL);
	lt_return_val_if_fail (tmpl->magic_code == MAGIC_CODE, NULL);
	lt_return_val_if_fail (tmpl->init != NULL, NULL);

	retval = tmpl->init(tmpl);
	if (retval)
		retval->target = tmpl;

	return retval;
}

/**
 * lt_iter_finish:
 * @iter: a #lt_iter_t
 *
 * Finalize the iterator and free its memory.
 */
void
lt_iter_finish(lt_iter_t *iter)
{
	lt_return_if_fail (iter != NULL);
	lt_return_if_fail (iter->target != NULL);

	iter->target->fini(iter);

	free(iter);
}

/**
 * lt_iter_next:
 * @iter: a #lt_iter_t
 * @key: (allow-none): a location to store the key, or %NULL
 * @val: (allow-none): a location to store the value, or %NULL
 *
 * Advances @iter and retrieves the key and/or value that are now
 * pointed to as a result of this advancement. If %FALSE is returned,
 * @key and @value are not set, and the iterator becomes invalid.
 *
 * Returns: %FALSE if the end of the object that the iterator is adapted
 *          has been reached.
 */
lt_bool_t
lt_iter_next(lt_iter_t    *iter,
	     lt_pointer_t *key,
	     lt_pointer_t *val)
{
	lt_return_val_if_fail (iter != NULL, FALSE);
	lt_return_val_if_fail (iter->target != NULL, FALSE);
	lt_return_val_if_fail (iter->target->next != NULL, FALSE);

	return iter->target->next(iter, key, val);
}
