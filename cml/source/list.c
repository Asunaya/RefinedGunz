/*
	list.c
	------

	Linked List Module

	Programming by Chojoongpil
	All copyright (c) 1997, MAIET entertainment software
*/
#include <stdio.h>
#include <stdlib.h>
#include "cml.h"

typedef struct ListNode {
    struct ListNode *prev;	/* Link to previous node in list. */
    struct ListNode *next;	/* Link to next node in list. */
    void *data;				/* Data value for item I. */
} ListNode;

/*
	ListAdd

	리스트내에 해당 아이템을 더한다.
	position - 데이터가 더해질 위치. 단, position의 값이 -1이라면 리스트의 맨 뒤에
	아이템이 더해진다.
*/
int ListAdd( LIST *list, int position, void *item )
{
	ListNode *node, *prev;

/* Create a list node. */
    node = (ListNode *) malloc(sizeof (ListNode));
    if (node == NULL){
#if (_DEBUG) & (_CONSOLE)
		printf("(ListAdd) Cannot allocate memory for linked list node.\n");
#endif
		return 0;
    }
    node->prev = node->next = NULL;
    node->data = item;

	/* Add the item to the list. */
    prev = *list;
    if(prev == NULL){
        node->prev = node;
        *list = node;
    } else {					/* Existing list? */
        if(position < 0){		/* End of list? */
            node->next = prev;  prev = prev->prev;
            node->prev = prev;  prev->next = node;
            prev = node->next;  prev->prev = node;
            node->next = NULL;
        } else if(position == 0){
								/* Beginning of list? */
            node->next = prev;  node->prev = prev->prev;
            prev->prev = node;  *list = node;
        } else{					/* Position I in list? */
            while ((--position > 0) && (prev->next != NULL)) prev = prev->next;
            node->prev = prev;  node->next = prev->next;  prev->next = node;
            if (node->next == NULL) (*list)->prev = node;
            else (node->next)->prev = node;
        }
    }
    return 1;
}

/*
	ListRemove

	List내에서 해당 위치의 아이템을 삭제한다.
*/

void *ListRemove( LIST *list, int position )
{
	ListNode *node, *prev;
	void *data;

    node = *list;
    if((node == NULL) || (position < 1)){
        return (NULL);
    }else if (position == 1){			/* Item 1 in list? */
        *list = node->next;
        if(node->next != NULL) (node->next)->prev = node->prev;
    } else {							/* Item 2..N in list? */
		while ((--position > 0) && (node != NULL))
			node = node->next;
		if (node == NULL)  return (NULL);
		prev = node->prev;  prev->next = node->next;
		if (node->next == NULL)			/* Very last item in list? */
			(*list)->prev = prev;
		else							/* Middle of list? */
			(node->next)->prev = prev;
    }
    data = node->data;
    free ((char *) node);
    return (data);
}

/*
	ListFind

	List에서 해당 아이템이 있는지 검색한다. (반환값으로 아이템의 위치값을 인덱스로 반환한다.)
*/
int ListFind( LIST list, void *item )
{
	int i;
	
    for (i = 1 ;  list != NULL ;  list = list->next, i++)
		if (list->data == item)  break ;

    return ((list == NULL) ? 0 : i);
}

/*
	ListGet

	LIST에서 해당 포지션에 위치한 데이터를 가져온다.
	데이터가 void *형으로 반환되어 진다.
*/
void *ListGet( LIST list, int position )
{
	if( list == NULL ){
		return (NULL);
	} else if( position < 0 ){
        return ((list->prev)->data);
	} else if(position == 0){
        return (NULL);
	}

    while((--position > 0) && (list != NULL)) list = list->next;
    if (list == NULL)	/* I > N */
        return (NULL);
    else				/* 1 <= I <= N */
        return (list->data);
}

/*
	ListGetCount()

	List의 아이템 갯수를 구한다.
*/
int ListGetCount( LIST list )
{
	int  count ;

    for(count = 0; list != NULL;  count++) list = list->next;

    return (count);
}
