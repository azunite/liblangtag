/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * lt-mem.c
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
#include <string.h>
#include "lt-config.h"
#include "lt-atomic.h"
#include "lt-mem.h"
#include "lt-messages.h"

struct _lt_mem_slist_t {
	lt_mem_slist_t    *next;
	lt_pointer_t       key;
	lt_destroy_func_t  func;
};
struct _lt_mem_serialized_t {
	uint32_t magic_code;
	char     byte_order;
	char     pad1[3];
	uint32_t total_size;
	uint32_t nmemb;
	uint32_t offset[FLEXIBLE_ARRAY_MEMBER];
};
struct _lt_mem_serialized_data_t {
	char         type;
	char         pad1[3];
	uint32_t     size;
	lt_pointer_t data[FLEXIBLE_ARRAY_MEMBER];
};

lt_mem_slist_t *lt_mem_slist_new        (void);
void            lt_mem_slist_free       (lt_mem_slist_t     *list);
lt_mem_slist_t *lt_mem_slist_last       (lt_mem_slist_t     *list);
lt_mem_slist_t *lt_mem_slist_append     (lt_mem_slist_t     *list,
                                         lt_pointer_t        key,
                                         lt_destroy_func_t   func);
lt_mem_slist_t *lt_mem_slist_delete     (lt_mem_slist_t     *list,
                                         lt_pointer_t       *data);
lt_mem_slist_t *lt_mem_slist_delete_link(lt_mem_slist_t     *list,
                                         lt_mem_slist_t     *link_);
lt_mem_slist_t *lt_mem_slist_find       (lt_mem_slist_t     *list,
                                         const lt_pointer_t  data);

static int _lt_mem_byte_order = 0;
static const lt_mem_serializer_funcs_t *_lt_mem_serializer_funcs[LT_MEM_SERIALIZER_TYPE_END];

LT_LOCK_DEFINE_STATIC (serializer);

/*< private >*/

#define DEFUNC_SERIALIZER_FUNCS(_t_, _T_)				\
	static lt_mem_serializer_type_t					\
	_lt_mem_ ## _t_ ## _get_type(void)				\
	{								\
		return LT_MEM_SERIALIZER_TYPE_ ## _T_;			\
	}								\
									\
	static uint32_t							\
	_lt_mem_ ## _t_ ## _get_size(const lt_pointer_t data)		\
	{								\
		return SIZEOF_ ## _T_ ## _T;				\
	}								\
									\
	static lt_bool_t						\
	_lt_mem_ ## _t_ ## _serialize(lt_mem_serialized_data_t *object,	\
				      const lt_pointer_t        data)	\
	{								\
		int i;							\
		_t_ ## _t n;						\
									\
		n = (_t_ ## _t)(long)data;				\
		n = lt_mem_serialize_ ## _t_ (n);			\
		for (i = 0; i < SIZEOF_ ## _T_ ## _T; i++) {		\
			((char *)object->data)[i] = ((char *)&n)[i];	\
		}							\
									\
		return TRUE;						\
	}								\
									\
	static lt_pointer_t						\
	_lt_mem_ ## _t_ ## _deserialize(const lt_mem_serialized_data_t *data, \
					char                            byte_order) \
	{								\
		_t_ ## _t retval;					\
		int i;							\
									\
		for (i = 0; i < SIZEOF_ ## _T_ ## _T; i++) {		\
			((char *)&retval)[i] = ((char *)data->data)[i];	\
		}							\
		return LT_INT_TO_POINTER (lt_mem_deserialize_ ## _t_ (retval, byte_order)); \
	}								\
									\
	static const lt_mem_serializer_funcs_t *			\
	_lt_mem_serializer_get_ ## _t_ ## _serializer_funcs(void)	\
	{								\
		static const lt_mem_serializer_funcs_t retval = {	\
			_lt_mem_ ## _t_ ## _get_type,			\
			_lt_mem_ ## _t_ ## _get_size,			\
			_lt_mem_ ## _t_ ## _serialize,			\
			_lt_mem_ ## _t_ ## _deserialize			\
		};							\
									\
		return &retval;						\
	}

DEFUNC_SERIALIZER_FUNCS (int8, INT8)
DEFUNC_SERIALIZER_FUNCS (uint8, INT8)
DEFUNC_SERIALIZER_FUNCS (int16, INT16)
DEFUNC_SERIALIZER_FUNCS (uint16, INT16)
DEFUNC_SERIALIZER_FUNCS (int32, INT32)
DEFUNC_SERIALIZER_FUNCS (uint32, INT32)
DEFUNC_SERIALIZER_FUNCS (int64, INT64)
DEFUNC_SERIALIZER_FUNCS (uint64, INT64)

static lt_mem_serializer_type_t
_lt_mem_string_get_type(void)
{
	return LT_MEM_SERIALIZER_TYPE_STRING;
}

static uint32_t
_lt_mem_string_get_size(const lt_pointer_t data)
{
	return LT_ALIGNED_TO_POINTER (strlen(data) + 1);
}

static lt_bool_t
_lt_mem_string_serialize(lt_mem_serialized_data_t *object,
			 const lt_pointer_t        data)
{
	int i;
	size_t len = strlen(data);

	for (i = 0; i < len; i++) {
		((char *)object->data)[i] = ((char *)data)[i];
	}
	((char *)object->data)[i] = 0;

	return TRUE;
}

static lt_pointer_t
_lt_mem_string_deserialize(const lt_mem_serialized_data_t *data,
			   char                            byte_order)
{
	size_t size = data->size - sizeof (lt_mem_serialized_data_t);
	char *p = malloc(size);
	int i;

	if (p) {
		for (i = 0; i < size && ((char *)data->data)[i] != 0; i++) {
			p[i] = ((char *)data->data)[i];
		}
		p[i] = 0;
	}

	return p;
}

static const lt_mem_serializer_funcs_t *
_lt_mem_serializer_get_string_serializer_funcs(void)
{
	static const lt_mem_serializer_funcs_t retval = {
		_lt_mem_string_get_type,
		_lt_mem_string_get_size,
		_lt_mem_string_serialize,
		_lt_mem_string_deserialize
	};

	return &retval;
}

/*< protected >*/
lt_mem_slist_t *
lt_mem_slist_new(void)
{
	return malloc(sizeof (lt_mem_slist_t));
}

void
lt_mem_slist_free(lt_mem_slist_t *list)
{
	lt_mem_slist_t *l = list;

	while (l) {
		list = l;
		l = l->next;
		free(list);
	}
}

lt_mem_slist_t *
lt_mem_slist_last(lt_mem_slist_t *list)
{
	if (list) {
		while (list->next)
			list = list->next;
	}

	return list;
}

lt_mem_slist_t *
lt_mem_slist_append(lt_mem_slist_t    *list,
		    lt_pointer_t       key,
		    lt_destroy_func_t  func)
{
	lt_mem_slist_t *l = lt_mem_slist_new();
	lt_mem_slist_t *last;

	l->key = key;
	l->func = func;
	l->next = NULL;
	if (list) {
		last = lt_mem_slist_last(list);
		last->next = l;
	} else {
		list = l;
	}

	return list;
}

lt_mem_slist_t *
lt_mem_slist_delete(lt_mem_slist_t *list,
		    lt_pointer_t   *data)
{
	lt_mem_slist_t *l = list;

	while (l) {
		if (l->key == data) {
			list = lt_mem_slist_delete_link(list, l);
			break;
		} else {
			l = l->next;
		}
	}

	return list;
}

lt_mem_slist_t *
lt_mem_slist_delete_link(lt_mem_slist_t *list,
			 lt_mem_slist_t *link_)
{
	lt_mem_slist_t *prev = NULL, *l = list;

	while (l) {
		if (l == link_) {
			if (prev)
				prev->next = l->next;
			if (list == l)
				list = list->next;
			free(link_);
			break;
		}
		prev = l;
		l = l->next;
	}

	return list;
}

lt_mem_slist_t *
lt_mem_slist_find(lt_mem_slist_t     *list,
		  const lt_pointer_t  data)
{
	while (list) {
		if (list->key == data)
			break;
		list = list->next;
	}

	return list;
}

/*< public >*/
char
lt_mem_get_byte_order(void)
{
	char retval;
	int i = 1;
	char *p;

	if (lt_atomic_int_get(&_lt_mem_byte_order) != 0)
		return _lt_mem_byte_order;

	p = (char *)&i;
	if (*p == 1) {
		/* We are on LSB */
		retval = LT_MEM_LSB;
	} else {
		/* We are on MSB */
		retval = LT_MEM_MSB;
	}
	lt_atomic_int_set(&_lt_mem_byte_order, retval);

	return retval;
}

void
lt_mem_serializer_init(void)
{
#define F(_t_, _T_)							\
	lt_mem_register_serializer(LT_MEM_SERIALIZER_TYPE_ ## _T_,	\
				   _lt_mem_serializer_get_ ## _t_ ## _serializer_funcs());

	F (int8, INT8);
	F (uint8, INT8);
	F (int16, INT16);
	F (uint16, INT16);
	F (int32, INT32);
	F (uint32, INT32);
	F (int64, INT64);
	F (uint64, INT64);
	F (string, STRING);

#undef F
}

size_t
lt_mem_get_object_size(lt_mem_t *object)
{
	lt_return_val_if_fail (object != NULL, 0);

	return object->size;
}

lt_pointer_t
lt_mem_alloc_object(size_t size)
{
	lt_mem_t *retval;

	lt_return_val_if_fail (size > 0, NULL);

	retval = calloc(1, size);
	if (retval) {
		retval->magic_code = LT_MEM_MAGIC_CODE;
		retval->ref_count = 1;
		retval->type = LT_MEM_SERIALIZER_TYPE_0;
		retval->size = size;
	}

	return retval;
}

lt_pointer_t
lt_mem_ref(lt_mem_t *object)
{
	lt_return_val_if_fail (object != NULL, NULL);

	lt_atomic_int_inc((volatile int *)&object->ref_count);

	return object;
}

void
lt_mem_unref(lt_mem_t *object)
{
	lt_return_if_fail (object != NULL);

	if (lt_atomic_int_dec_and_test((volatile int *)&object->ref_count)) {
		lt_mem_slist_t *ll, *l;

		if (object->refs) {
			ll = object->refs;
			while (ll) {
				l = ll;
				ll = ll->next;
				if (l->func)
					l->func(l->key);
				free(l);
			}
		}
		if (object->weak_pointers) {
			ll = object->weak_pointers;
			while (ll) {
				lt_pointer_t *p;

				l = ll;
				ll = ll->next;
				p = (lt_pointer_t *)l->key;
				*p = NULL;
				free(l);
			}
		}
		free(object);
	}
}

void
lt_mem_add_ref(lt_mem_t          *object,
	       lt_pointer_t       p,
	       lt_destroy_func_t  func)
{
	lt_return_if_fail (object != NULL);
	lt_return_if_fail (p != NULL);
	lt_return_if_fail (func != NULL);

	object->refs = lt_mem_slist_append(object->refs, p, func);
}

void
lt_mem_remove_ref(lt_mem_t     *object,
		  lt_pointer_t  p)
{
	lt_mem_slist_t *l;

	lt_return_if_fail (object != NULL);
	lt_return_if_fail (p != NULL);

	if ((l = lt_mem_slist_find(object->refs, p)) != NULL) {
		object->refs = lt_mem_slist_delete_link(object->refs, l);
	}
}

void
lt_mem_delete_ref(lt_mem_t     *object,
		  lt_pointer_t  p)
{
	lt_mem_slist_t *l;

	lt_return_if_fail (object != NULL);
	lt_return_if_fail (p != NULL);

	if ((l = lt_mem_slist_find(object->refs, p)) != NULL) {
		if (l->func)
			l->func(l->key);
		object->refs = lt_mem_slist_delete_link(object->refs, l);
	}
}

void
lt_mem_add_weak_pointer(lt_mem_t     *object,
			lt_pointer_t *p)
{
	lt_return_if_fail (object != NULL);
	lt_return_if_fail (p != NULL);

	if (!lt_mem_slist_find(object->weak_pointers, p))
		object->weak_pointers = lt_mem_slist_append(object->weak_pointers, p, NULL);
}

void
lt_mem_remove_weak_pointer(lt_mem_t     *object,
			   lt_pointer_t *p)
{
	lt_return_if_fail (object != NULL);
	lt_return_if_fail (p != NULL);

	object->weak_pointers = lt_mem_slist_delete(object->weak_pointers, p);
}

lt_bool_t
lt_mem_is_object(lt_pointer_t data)
{
	uint32_t *magic = (uint32_t *)data;

	if (!data)
		return FALSE;

	return *magic == LT_MEM_MAGIC_CODE;
}

lt_bool_t
lt_mem_is_serialized_object(lt_pointer_t data)
{
	uint32_t *magic = (uint32_t *)data, i;
	char bo;

	if (!data)
		return FALSE;

	bo = *((char *)(magic + 1));
	i = *magic;
	return lt_mem_deserialize_uint32(i, bo) == LT_MEM_SERIALIZED_CODE;
}

void
lt_mem_register_serializer(lt_mem_serializer_type_t         type,
			   const lt_mem_serializer_funcs_t *funcs)
{
	lt_return_if_fail (type > LT_MEM_SERIALIZER_TYPE_0);
	lt_return_if_fail (type < LT_MEM_SERIALIZER_TYPE_END);
	lt_return_if_fail (funcs != NULL);

	_lt_mem_serializer_funcs[type] = funcs;
}

lt_mem_serialized_t *
lt_mem_alloc_serialized_object(size_t nmemb)
{
	lt_mem_serialized_t *retval;
	uint32_t magic_code = LT_MEM_SERIALIZED_CODE;
	size_t n;

	lt_return_val_if_fail (nmemb > 0, NULL);

	n = LT_ALIGNED_TO_POINTER (sizeof (lt_mem_serialized_t) + (sizeof (uint32_t)) * nmemb);
	retval = calloc(1, n);
	if (retval) {
		retval->byte_order = lt_mem_get_byte_order();
		retval->magic_code = lt_mem_serialize_uint32 (magic_code);
		retval->nmemb      = lt_mem_serialize_uint32 (nmemb);
		retval->total_size = lt_mem_serialize_uint32 (n);
	}

	return retval;
}

lt_mem_serialized_data_t *
lt_mem_serialized_get_pointer(lt_mem_serialized_t      *object,
			      uint32_t                  pos,
			      lt_mem_serializer_type_t  type,
			      const lt_pointer_t        data)
{
	lt_mem_serialized_data_t *retval = NULL;
	uint32_t nmemb, size, total_size;

	lt_return_val_if_fail (lt_mem_is_serialized_object(object), NULL);
	lt_return_val_if_fail (type > LT_MEM_SERIALIZER_TYPE_0, NULL);
	lt_return_val_if_fail (type < LT_MEM_SERIALIZER_TYPE_END, NULL);

	LT_LOCK (serializer);

	total_size = lt_mem_deserialize_uint32(object->total_size, object->byte_order);
	nmemb = lt_mem_deserialize_uint32(object->nmemb, object->byte_order);
	if (pos >= nmemb) {
		lt_pointer_t p;

		size = sizeof (uint32_t) * (pos - nmemb + 1);
		p = realloc(object, object->total_size + size);
		if (!p)
			return NULL;
		object = p;
		object->total_size = lt_mem_serialize_uint32(total_size + size);
		memset(&object->offset[nmemb], 0, size);
		object->nmemb = lt_mem_serialize_uint32(nmemb + 1);
	}
	if (object->offset[pos] == 0) {
		const lt_mem_serializer_funcs_t *serializer = lt_mem_get_serializer_funcs(type);
		lt_pointer_t p;

		if (!serializer)
			goto bail;
		size = LT_ALIGNED_TO_POINTER (sizeof (lt_mem_serialized_data_t) + serializer->get_size(data));
		p = realloc(object, total_size + size);
		if (!p)
			goto bail;
		object = p;
		object->offset[pos] = lt_mem_serialize_uint32(total_size);
		retval = (lt_mem_serialized_data_t *)(object + total_size);
		total_size += size;
		object->total_size = lt_mem_serialize_uint32(total_size);

		retval->type = type;
		retval->size = lt_mem_serialize_uint32(size);
	} else {
		retval = (lt_mem_serialized_data_t *)(object + lt_mem_deserialize_uint32(object->offset[pos], object->byte_order));
		if (retval->type != type) {
			lt_warning("object type at the position %d is difference: expected %d, but %d",
				   pos, type, retval->type);
			retval = NULL;
			goto bail;
		}
	}
  bail:
	LT_UNLOCK (serializer);

	return retval;
}

void
lt_mem_free_serialized_object(lt_mem_serialized_t *object)
{
	lt_return_if_fail (lt_mem_is_serialized_object(object));

	if (object)
		free(object);
}

lt_bool_t
lt_mem_serialize(lt_mem_serialized_t      *object,
		 uint32_t                  pos,
		 lt_mem_serializer_type_t  type,
		 const lt_pointer_t        data)
{
	lt_mem_serialized_data_t *serialized;
	const lt_mem_serializer_funcs_t *serializer;

	lt_return_val_if_fail (lt_mem_is_serialized_object(object), FALSE);
	lt_return_val_if_fail (data != NULL, FALSE);
	lt_return_val_if_fail (type > LT_MEM_SERIALIZER_TYPE_0, FALSE);
	lt_return_val_if_fail (type < LT_MEM_SERIALIZER_TYPE_END, FALSE);

	serializer = lt_mem_get_serializer_funcs(type);
	if (!serializer)
		return FALSE;

	serialized = lt_mem_serialized_get_pointer(object, pos, type, data);

	return serializer->serialize(serialized, data);
}

lt_pointer_t
lt_mem_deserialize(lt_mem_serialized_t *data)
{
	const lt_mem_serializer_funcs_t *serializer;

	lt_return_val_if_fail (data != NULL, NULL);

	serializer = lt_mem_get_serializer_funcs(data->type);

	return serializer->deserialize(data, data->byte_order);
}

const lt_mem_serializer_funcs_t *
lt_mem_get_serializer_funcs(lt_mem_serializer_type_t type)
{
	lt_return_val_if_fail (type > LT_MEM_SERIALIZER_TYPE_0, NULL);
	lt_return_val_if_fail (type < LT_MEM_SERIALIZER_TYPE_END, NULL);

	return _lt_mem_serializer_funcs[type];
}

const lt_mem_serializer_funcs_t *
lt_mem_get_object_serializer_funcs(lt_mem_t *object)
{
	lt_return_val_if_fail (object != NULL, NULL);

	return lt_mem_get_serializer_funcs(object->type);
}
