/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * lt-mem.h
 * Copyright (C) 2011-2012 Akira TAGOH
 * 
 * Authors:
 *   Akira TAGOH  <akira@tagoh.org>
 * 
 * You may distribute under the terms of either the GNU
 * Lesser General Public License or the Mozilla Public
 * License, as specified in the README file.
 */
#ifndef __LT_MEM_H__
#define __LT_MEM_H__

#include "lt-stdint.h"
#include "lt-macros.h"

LT_BEGIN_DECLS

#define LT_MEM_SERIALIZED_CODE		0xF4EE23D0
#define LT_MEM_MAGIC_CODE		0xDEADBEEF
#define LT_MEM_LSB			0x0f
#define LT_MEM_MSB			0xf0

#define lt_mem_serialize_uint8(_v_)	((uint8_t)(_v_))
#define lt_mem_serialize_int8(_v_)	((int8_t)(_v_))
#define lt_mem_serialize_uint16(_v_)			\
	(lt_mem_get_byte_order() == LT_MEM_MSB ?	\
	 LT_UINT16_TO_BE (_v_) : LT_UINT16_TO_LE (_v_))
#define lt_mem_serialize_int16(_v_)			\
	(lt_mem_get_byte_order() == LT_MEM_MSB ?	\
	 LT_INT16_TO_BE (_v_) : LT_INT16_TO_LE (_v_))
#define lt_mem_serialize_uint32(_v_)			\
	(lt_mem_get_byte_order() == LT_MEM_MSB ?	\
	 LT_UINT32_TO_BE (_v_) : LT_UINT32_TO_LE (_v_))
#define lt_mem_serialize_int32(_v_)			\
	(lt_mem_get_byte_order() == LT_MEM_MSB ?	\
	 LT_INT32_TO_BE (_v_) : LT_INT32_TO_LE (_v_))
#define lt_mem_serialize_uint64(_v_)			\
	(lt_mem_get_byte_order() == LT_MEM_MSB ?	\
	 LT_UINT64_TO_BE (_v_) : LT_UINT64_TO_LE (_v_))
#define lt_mem_serialize_int64(_v_)			\
	(lt_mem_get_byte_order() == LT_MEM_MSB ?	\
	 LT_INT64_TO_BE (_v_) : LT_INT64_TO_LE (_v_))
#define lt_mem_deserialize_uint8(_v_, _o_) ((uint8_t)(_v_))
#define lt_mem_deserialize_int8(_v_, _o_) ((int8_t)(_v_))
#define lt_mem_deserialize_uint16(_v_, _o_)	\
	(lt_mem_get_byte_order() != (_o_) ?	\
	 lt_mem_serialize_uint16(_v_) :		\
	 (_v_))
#define lt_mem_deserialize_int16(_v_, _o_)	\
	(lt_mem_get_byte_order() != (_o_) ?	\
	 lt_mem_serialize_int16(_v_) :		\
	 (_v_))
#define lt_mem_deserialize_uint32(_v_, _o_)	\
	(lt_mem_get_byte_order() != (_o_) ?	\
	 lt_mem_serialize_uint32(_v_) :		\
	 (_v_))
#define lt_mem_deserialize_int32(_v_, _o_)	\
	(lt_mem_get_byte_order() != (_o_) ?	\
	 lt_mem_serialize_int32(_v_) :		\
	 (_v_))
#define lt_mem_deserialize_uint64(_v_, _o_)	\
	(lt_mem_get_byte_order() != (_o_) ?	\
	 lt_mem_serialize_uint64(_v_) :		\
	 (_v_))
#define lt_mem_deserialize_int64(_v_, _o_)	\
	(lt_mem_get_byte_order() != (_o_) ?	\
	 lt_mem_serialize_int64(_v_) :		\
	 (_v_))

#define LT_UINT16_SWAP_LE_BE(_v_)			\
	((uint16_t)((uint16_t)((uint16_t)(_v_) >> 8) |	\
		    (uint16_t)((uint16_t)(_v_) << 8)))
#define LT_UINT32_SWAP_LE_BE(_v_)					\
	((uint32_t)(((uint32_t)((uint32_t)(_v_) & (uint32_t)0x000000ffU) << 24) | \
		    ((uint32_t)((uint32_t)(_v_) & (uint32_t)0x0000ff00U) <<  8) | \
		    ((uint32_t)((uint32_t)(_v_) & (uint32_t)0x00ff0000U) <<  8) | \
		    ((uint32_t)((uint32_t)(_v_) & (uint32_t)0xff000000U) << 24)))
#define LT_UINT64_SWAP_LE_BE(_v_)					\
	((uint64_t)((((uint64_t) (_v_) &				\
		      (uint64_t) LT_INT64_CONSTANT (0x00000000000000ffU)) << 56) | \
		    (((uint64_t) (_v_) &				\
		      (uint64_t) LT_INT64_CONSTANT (0x000000000000ff00U)) << 40) | \
		    (((uint64_t) (_v_) &				\
		      (uint64_t) LT_INT64_CONSTANT (0x0000000000ff0000U)) << 24) | \
		    (((uint64_t) (_v_) &				\
		      (uint64_t) LT_INT64_CONSTANT (0x00000000ff000000U)) <<  8) | \
		    (((uint64_t) (_v_) &				\
		      (uint64_t) LT_INT64_CONSTANT (0x000000ff00000000U)) >>  8) | \
		    (((uint64_t) (_v_) &				\
		      (uint64_t) LT_INT64_CONSTANT (0x0000ff0000000000U)) >> 24) | \
		    (((uint64_t) (_v_) &				\
		      (uint64_t) LT_INT64_CONSTANT (0x00ff000000000000U)) >> 40) | \
		    (((uint64_t) (_v_) &				\
		      (uint64_t) LT_INT64_CONSTANT (0xff00000000000000U)) >> 56)))


typedef struct _lt_mem_t			lt_mem_t;
typedef struct _lt_mem_serializer_funcs_t	lt_mem_serializer_funcs_t;
typedef struct _lt_mem_serialized_t		lt_mem_serialized_t;
typedef struct _lt_mem_serialized_data_t	lt_mem_serialized_data_t;
typedef struct _lt_mem_slist_t			lt_mem_slist_t;
typedef enum _lt_mem_serializer_type_t		lt_mem_serializer_type_t;


typedef lt_mem_serializer_type_t (* lt_mem_serializer_get_type_func_t) (void);

typedef uint32_t     (* lt_mem_serializer_get_size_func_t)    (const lt_pointer_t         data);
typedef lt_bool_t    (* lt_mem_serializer_serialize_func_t)   (lt_mem_serialized_data_t *object,
							       const lt_pointer_t        data);
typedef lt_pointer_t (* lt_mem_serializer_deserialize_func_t) (const lt_mem_serialized_data_t *data,
							       char                            byte_order);


enum _lt_mem_serializer_type_t {
	LT_MEM_SERIALIZER_TYPE_0 = 0,
	LT_MEM_SERIALIZER_TYPE_INT8,
	LT_MEM_SERIALIZER_TYPE_INT16,
	LT_MEM_SERIALIZER_TYPE_INT32,
	LT_MEM_SERIALIZER_TYPE_INT64,
	LT_MEM_SERIALIZER_TYPE_STRING,
	LT_MEM_SERIALIZER_TYPE_ARRAY,
	LT_MEM_SERIALIZER_TYPE_TRIE,
	LT_MEM_SERIALIZER_TYPE_LIST,
	LT_MEM_SERIALIZER_TYPE_END
};
struct _lt_mem_t {
	uint32_t                  magic_code;
	volatile unsigned int     ref_count;
	lt_mem_serializer_type_t  type;
	size_t                    size;
	lt_mem_slist_t           *refs;
	lt_mem_slist_t           *weak_pointers;
};
struct _lt_mem_serializer_funcs_t {
	lt_mem_serializer_get_type_func_t    get_type;
	lt_mem_serializer_get_size_func_t    get_size;
	lt_mem_serializer_serialize_func_t   serialize;
	lt_mem_serializer_deserialize_func_t deserialize;
};


char         lt_mem_get_byte_order     (void);
size_t       lt_mem_get_object_size    (lt_mem_t          *object);
lt_pointer_t lt_mem_alloc_object       (size_t             size);
lt_pointer_t lt_mem_ref                (lt_mem_t          *object);
void         lt_mem_unref              (lt_mem_t          *object);
void         lt_mem_add_ref            (lt_mem_t          *object,
                                        lt_pointer_t       p,
                                        lt_destroy_func_t  func);
void         lt_mem_remove_ref         (lt_mem_t          *object,
                                        lt_pointer_t       p);
void         lt_mem_delete_ref         (lt_mem_t          *object,
                                        lt_pointer_t       p);
void         lt_mem_add_weak_pointer   (lt_mem_t          *object,
                                        lt_pointer_t      *p);
void         lt_mem_remove_weak_pointer(lt_mem_t          *object,
                                        lt_pointer_t      *p);
lt_bool_t    lt_mem_is_object          (lt_pointer_t       data);

lt_bool_t                 lt_mem_is_serialized_object   (lt_pointer_t                     data);
void                      lt_mem_serializer_init        (void);
void                      lt_mem_register_serializer    (lt_mem_serializer_type_t         type,
                                                         const lt_mem_serializer_funcs_t *funcs);
lt_mem_serialized_t      *lt_mem_alloc_serialized_object(size_t                           nmemb);
lt_mem_serialized_data_t *lt_mem_serialized_get_pointer (lt_mem_serialized_t             *object,
                                                         uint32_t                         pos,
                                                         lt_mem_serializer_type_t         type,
                                                         const lt_pointer_t               data);
void                      lt_mem_free_serialized_object (lt_mem_serialized_t             *object);
uint32_t                  lt_mem_get_serialized_size    (lt_mem_serialized_t             *data);
lt_bool_t                 lt_mem_serialize              (lt_mem_serialized_t             *object,
							 uint32_t                         pos,
							 lt_mem_serializer_type_t         type,
							 const lt_pointer_t               data);
lt_mem_serialized_t      *lt_mem_serialize_object       (const lt_mem_t                  *object);
lt_pointer_t              lt_mem_deserialize            (lt_mem_serialized_t             *data);

const lt_mem_serializer_funcs_t *lt_mem_get_serializer_funcs       (lt_mem_serializer_type_t         type);
const lt_mem_serializer_funcs_t *lt_mem_get_object_serializer_funcs(lt_mem_t                        *object);

LT_END_DECLS

#endif /* __LT_MEM_H__ */
