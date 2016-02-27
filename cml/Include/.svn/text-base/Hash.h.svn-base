/*
	Hash.h
	------

	programming by Chojoongpil
	All copyright (c) 1997, MAIET entertainment software
*/
#ifndef __HASH_HEADER__
#define __HASH_HEADER__

#ifdef __cplusplus
extern "C" {
#endif


/* hash 함수 헤더 */
typedef struct _HashTable *HASHTABLE;

extern  int  HashCreate(int maxEntries, HASHTABLE *table);
extern  int  HashDestroy(HASHTABLE table);
extern  int  HashAdd(HASHTABLE table, const char *key, const void *data);
extern  int  HashRemove(HASHTABLE table, const char *key);
extern  int  HashSearch(HASHTABLE table, const char *key,void **data);


#ifdef __cplusplus
}
#endif

#endif /* __CML_HEADER__ */

/*
------------------------------------------------------------------
*/