#ifndef LINKLIST_H_INCLUDED
#define LINKLIST_H_INCLUDED

#include "inline.h"

struct list_head {
/*----------------------------------------------------------------------------
  This is a header for an element of a doubly linked list, or an anchor
  for such a list.

  itemP == NULL means it's an anchor; otherwise it's a header.

  Initialize a list header with list_init_header().  You don't have to
  do anything to terminate a list header.

  Initialize an anchor with list_make_emtpy().  You don't have to do anything
  to terminate a list header.
-----------------------------------------------------------------------------*/
    struct list_head * nextP;
        /* For a header, this is the address of the list header for
           the next element in the list.  If there is no next element,
           it points to the anchor.  If the header is not in a list at
           all, it is NULL.

           For an anchor, it is the address of the list header of the
           first element.  If the list is empty, it points to the
           anchor itself.  
        */
    struct list_head * prevP;
        /* For a header, this is the address of the list header for
           the previous element in the list.  If there is no previous element,
           it points to the anchor.  If the header is not in a list at
           all, it is NULL.

           For an anchor, it is the address of the list header of the
           last element.  If the list is empty, it points to the
           anchor itself.  
        */
    void * itemP;
        /* For a header, this is the address of the list element to which it
           belongs.  For an anchor, this is NULL.
        */
};

static __inline__ void
list_init_header(struct list_head * const headerP,
                 void *             const itemP) {

    headerP->prevP = NULL;
    headerP->nextP = NULL;
    headerP->itemP = itemP;
}



static __inline__ int
list_is_linked(struct list_head * headerP) {
    return headerP->prevP != NULL;
}



static __inline__ int
list_is_empty(struct list_head * const anchorP)  {
    return anchorP->nextP == anchorP;
}



static __inline__ unsigned int
list_count(struct list_head * const anchorP) {
    unsigned int count;

    struct list_head * p;

    for (p = anchorP->nextP, count = 0;
         p != anchorP;
         p = p->nextP, ++count);

    return count;
}



static __inline__ void
list_make_empty(struct list_head * const anchorP) {
    anchorP->prevP = anchorP;
    anchorP->nextP = anchorP;
    anchorP->itemP = NULL;
}

static __inline__ void
list_insert_after(struct list_head * const beforeHeaderP,
                  struct list_head * const newHeaderP) {
    newHeaderP->prevP = beforeHeaderP;
    newHeaderP->nextP = beforeHeaderP->nextP;

    beforeHeaderP->nextP = newHeaderP;
    newHeaderP->nextP->prevP = newHeaderP;
}



static __inline__ void
list_add_tail(struct list_head * const anchorP,
              struct list_head * const headerP) {
    list_insert_after(anchorP->prevP, headerP);
}



static __inline__ void
list_add_head(struct list_head * const anchorP,
              struct list_head * const headerP) {
    list_insert_after(anchorP, headerP);
}



static __inline__ void
list_remove(struct list_head * const headerP) {
    headerP->prevP->nextP = headerP->nextP;
    headerP->nextP->prevP = headerP->prevP;
    headerP->prevP = NULL;
    headerP->nextP = NULL;
}



static __inline__ struct list_head *
list_remove_head(struct list_head * const anchorP) {
    struct list_head * retval;

    if (list_is_empty(anchorP))
        retval = NULL;
    else {
        retval = anchorP->nextP;
        list_remove(retval);
    }            
    return retval;
}



static __inline__ struct list_head *
list_remove_tail(struct list_head * const anchorP) {
    struct list_head * retval;

    if (list_is_empty(anchorP))
        retval = NULL;
    else {
        retval = anchorP->prevP;
        list_remove(retval);
    }            
    return retval;
}



static __inline__ void *
list_foreach(struct list_head * const anchorP,
             void * functionP(struct list_head * const, void * const),
             void *             const context) {

    struct list_head * p;
    struct list_head * nextP;
    void * result;

    for (p = anchorP->nextP, nextP = p->nextP, result=NULL;
         p != anchorP && result == NULL; 
         p = nextP, nextP = p->nextP) 
        result = (*functionP)(p, context);

    return result;
}



static __inline__ void
list_append(struct list_head * const newAnchorP,
            struct list_head * const baseAnchorP) {

    if (!list_is_empty(newAnchorP)) {
        baseAnchorP->prevP->nextP = newAnchorP->nextP;
        newAnchorP->nextP->prevP = baseAnchorP->prevP;
        newAnchorP->prevP->nextP = baseAnchorP;
        baseAnchorP->prevP = newAnchorP->prevP;
    }
}

#endif


