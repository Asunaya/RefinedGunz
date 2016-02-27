/*
	hash.c
	------

	hash table module.

	Programming by Chojoongpil
*/
#include "stdafx.h"
#include "Hash.h"	// common library header

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    Hash Table Data Structures.
*/

typedef struct HashItem {
	char				*key;	/* Item key. */
	const	void		*value;	/* Item value. */
	struct	HashItem	*next;	/* Pointer to next item in list. */
}  HashItem ;

typedef  struct  _HashTable {
    int  maxChains;			/* Maximum number of entries N in table. */
    int  numChains;			/* Actual number of non-empty entries. */
    int  longestChain;		/* Records length of longest chain. */
    HashItem  *chain[1];	/* Array of N pointers to item chains. */
}  _HashTable ;

/* Private Use Only Function Prototype */
static int hashKey( const  char  *key, int  tableSize );
static int hashPrime( int  number );

/*
	HashAdd

	한개의 element를 추가한다.
*/
int	HashAdd( HASHTABLE table, const char *key, const void *data )
{
	HashItem *item, *prev;
	int comparison, index;

	if (table == NULL) return 0;

	/* If the key is already in the hash table, then replace its data value. */
	index = hashKey (key, table->maxChains);
	comparison = -1;
	prev = (HashItem *) NULL;
    for(item = table->chain[index]; item != NULL; item = item->next){
		comparison = strcmp(item->key, key);
		if (comparison >= 0) break;
		prev = item;
	}

	if(comparison == 0){
		item->value = data;
#if (_DEBUG) & (_CONSOLE) & (_CMLDEBUG)
        printf ("(HashAdd) Replaced \"%s\":%p (%p) in table %p[%d].\n", key, data, item, table, index) ;
#endif
		return 1;
    }

/* Add a brand new item to the hash table: allocate an ITEM node for the item,
   fill in the fields, and link the new node into the chain of items. */
    item = (HashItem *) malloc(sizeof (HashItem));	// 아이템 할당
    if(item == NULL) return 0;

    item->key = strdup(key);	/* Fill in the item node. */
	if(item->key == NULL){
		free ((char *) item);
		return 0;
	}
    item->value = data;

    if(prev == NULL){			/* Link in at head of list. */
        item->next = table->chain[index];
        if(item->next == NULL) table->numChains++;
        table->chain[index] = item;
    }else{					/* Link in further down the list. */
		item->next = prev->next;
		prev->next = item;
	}

#if (_DEBUG) & (_CONSOLE) & (_CMLDEBUG)
	printf ("(HashAdd) Added \"%s\":%p (%p) to table %p[%d].\n", key, data, item, table, index) ;
#endif

/* For statistical purposes, measure the length of the chain and, if necessary,
   update the LONGEST_CHAIN value for the hash table. */
    comparison = 0 ;

    for(item = table->chain[index] ;  item != NULL ;  item = item->next) comparison++ ;

    if (table->longestChain < comparison) table->longestChain = comparison;
    
	return 1;
}

/*
	HashCreate

	해쉬 테이블 생성함수. (TRUE or FALSE 리턴)
*/
int HashCreate( int maxEntries, HASHTABLE *table )
{
    int i, prime, size;

/* Find the first prime number larger than the expected number of entries
   in the table. */
    prime = (maxEntries % 2) ? maxEntries : maxEntries + 1;
    for ( ; ; ) {			/* Check odd numbers only. */
        if (hashPrime (prime))  break;
        prime += 2;
    }

/* Create and initialize the hash table. */
    size = sizeof (_HashTable)  +  ((prime - 1) * sizeof (HashItem *));
    *table = (HASHTABLE) malloc (size);
    if (*table == NULL) return 0;

    (*table)->maxChains = prime;
    (*table)->numChains = 0;		/* Number of non-empty chains. */
    (*table)->longestChain = 0;	/* Length of longest chain. */
    
	for (i = 0; i < prime; i++) (*table)->chain[i] = (HashItem *) NULL;

#if (_DEBUG) & (_CONSOLE) & (_CMLDEBUG)
    printf ("(HashCreate) Created hash table %p of %d elements.\n", *table, prime);
#endif
    return 1;
}

/*
	HashRemove

	HashRemove 해쉬 테이블에서 키와 함께 데이터를 삭제한다.
*/
int HashRemove( HASHTABLE table, const char *key)
{
	HashItem *item, *prev;
	int index;

	if (table == NULL) return 0;

/* Locate the key's entry in the hash table. */
    index = hashKey (key, table->maxChains);

    prev = (HashItem *) NULL;

    for (item = table->chain[index]; item != NULL; item = item->next) {
        if (strcmp (item->key, key) == 0) break;
        prev = item;
    }

/* Unlink the entry from the hash table and free it. */
    if (item == NULL) {
#if (_DEBUG) & (_CONSOLE) & (_CMLDEBUG)
		printf ("(HashRemove) Key \"%s\" not found in table %p.\n", key, table);
#endif
		return (0);
	} else {
		if (prev == NULL)
			table->chain[index] = item->next;
		else
			prev->next = item->next;
	}
#if (_DEBUG) & (_CONSOLE) & (_CMLDEBUG)
	printf ("(HashRemove) Removed \"%s\":%p from table %p.\n", item->key, item->value, table);
#endif
    free (item->key);			/* Free item key. */
    free ((char *) item);		/* Free the item. */

    return (1) ;
}

/*
	HashDestroy

	해쉬 테이블을 삭제한다.
*/
int HashDestroy( HASHTABLE  table )
{
    int i;
    HashItem *item, *next;

#if (_DEBUG) & (_CONSOLE) & (_CMLDEBUG)
	printf ("(HashDestroy) Deleting hash table %p.\n", table);
#endif
    if(table == NULL) return 0;	// return FALSE

    for(i = 0; i < table->maxChains; i++){
        for(item = table->chain[i]; item != NULL; item = next){
			next = item->next;
			free(item->key);
            free((char *)item);
        }
    }
    free ((char *) table);

    return 1;	// return TRUE
}

/*
	hashKey

	키에서 해쉬 수를 구한다.
*/
static int hashKey( const  char  *key, int  tableSize )
{
    const char *s;
    unsigned int i, value, sum;

    if (tableSize == 0)  return (0) ;	/* Empty table? */

/* Fold the character string key into an integer number. */
#define  BITS_TO_SHIFT  8
    s = key;
    for (sum = 0 ;  *s != '\0' ; ) {
        for (i = value = 0 ;  (i < sizeof (int)) && (*s != '\0') ;  i++, s++)
            value = (value << BITS_TO_SHIFT) + *((unsigned char *) s);
        sum = sum + value;
    }
    
	return (sum % tableSize);		/* Return index [0..M-1] into table. */
}

/*
	hashPrime()

	해당 수가 prime수인지를 판별한다.
*/
static int hashPrime( int number )
{
	int divisor;

	if (number < 0)  number = -number;
	if (number < 4)  return (1);	/* 0, 1, 2, and 3 are prime. */

/*	Check for possible divisors.  The "divisor > dividend" test is similar
	to checking 2 .. sqrt(N) as possible divisors, but avoids the need for
	linking to the math library. */

	for(divisor = 2 ;  ;  divisor++){
		if((number % divisor) == 0) return (0);
		if(divisor > (number / divisor)) return (1);
	}
}

/*
	HashSearch
	----------

	table	: 해쉬 테이블
	key		: KEY string
	data	: 

	해쉬 테이블에서 주어진 키에 해당하는 데이터를 찾아 낸다.
*/
int HashSearch( HASHTABLE table, const char *key, void **data )
{
	HashItem *item;
	int comparison, index;
	
	/* 해쉬 테이블에서 데이터를 검색한다. */
    index = hashKey (key, table->maxChains);
    comparison = -1;
    for(item = table->chain[index];  item != NULL;  item = item->next){
        comparison = strcmp (item->key, key);
        if (comparison >= 0) break;
    }

	/* 결과치 반환 */
    if (comparison == 0) {
        if (data != NULL)  *data = (void *) item->value;
#if (_DEBUG) & (_CONSOLE) & (_CMLDEBUG)
        printf ("(HashSearch) \"%s\":%p found in table %p.\n", key, item->value, table);
#endif
		return (1);	// return TRUE
    } else {
        if (data != NULL)  *data = NULL;
#if (_DEBUG) & (_CONSOLE) & (_CMLDEBUG)
        printf ("(HashSearch) Key \"%s\" not found in table %p.\n", key, table);
#endif
        return (0) ;
    }
}

