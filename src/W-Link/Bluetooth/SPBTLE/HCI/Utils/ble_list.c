/******************** (C) COPYRIGHT 2012 STMicroelectronics ********************
* File Name          : ble_list.c
* Author             : AMS - HEA&RF BU
* Version            : V1.0.0
* Date               : 19-July-2012
* Description        : Circular Linked List Implementation.
*******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "NeonRTOS.h"

#include "ble_list.h"

#ifdef __cplusplus
extern "C" {
#endif

void list_init_head(tListNode *listHead)
{
    listHead->next = listHead;
    listHead->prev = listHead;
}

bool list_is_empty(tListNode *listHead)
{
    bool return_value;

    NeonRTOS_EnterCritical();

    return_value = (listHead->next == listHead);

    NeonRTOS_ExitCritical(0);

    return return_value;
}

void list_insert_head(tListNode *listHead, tListNode *node)
{
    NeonRTOS_EnterCritical();

    node->next = listHead->next;
    node->prev = listHead;

    listHead->next->prev = node;
    listHead->next = node;

    NeonRTOS_ExitCritical(0);
}

void list_insert_tail(tListNode *listHead, tListNode *node)
{
    NeonRTOS_EnterCritical();

    node->next = listHead;
    node->prev = listHead->prev;

    listHead->prev->next = node;
    listHead->prev = node;

    NeonRTOS_ExitCritical(0);
}

void list_remove_node(tListNode *node)
{
    NeonRTOS_EnterCritical();

    node->prev->next = node->next;
    node->next->prev = node->prev;

    node->next = NULL;
    node->prev = NULL;

    NeonRTOS_ExitCritical(0);
}

void list_remove_head(tListNode *listHead, tListNode **node)
{
    NeonRTOS_EnterCritical();

    if (listHead->next == listHead)
    {
        *node = NULL;
    }
    else
    {
        *node = listHead->next;

        listHead->next = (*node)->next;
        (*node)->next->prev = listHead;

        (*node)->next = NULL;
        (*node)->prev = NULL;
    }

    NeonRTOS_ExitCritical(0);
}

void list_remove_tail(tListNode *listHead, tListNode **node)
{
    NeonRTOS_EnterCritical();

    if (listHead->prev == listHead)
    {
        *node = NULL;
    }
    else
    {
        *node = listHead->prev;

        listHead->prev = (*node)->prev;
        (*node)->prev->next = listHead;

        (*node)->next = NULL;
        (*node)->prev = NULL;
    }

    NeonRTOS_ExitCritical(0);
}

void list_insert_node_after(tListNode *node, tListNode *ref_node)
{
    NeonRTOS_EnterCritical();

    node->next = ref_node->next;
    node->prev = ref_node;

    ref_node->next->prev = node;
    ref_node->next = node;

    NeonRTOS_ExitCritical(0);
}

void list_insert_node_before(tListNode *node, tListNode *ref_node)
{
    NeonRTOS_EnterCritical();

    node->next = ref_node;
    node->prev = ref_node->prev;

    ref_node->prev->next = node;
    ref_node->prev = node;

    NeonRTOS_ExitCritical(0);
}

int list_get_size(tListNode *listHead)
{
    int size = 0;
    tListNode *temp;

    NeonRTOS_EnterCritical();

    temp = listHead->next;

    while (temp != listHead)
    {
        size++;
        temp = temp->next;
    }

    NeonRTOS_ExitCritical(0);

    return size;
}

void list_get_next_node(tListNode *ref_node, tListNode **node)
{
    NeonRTOS_EnterCritical();

    *node = ref_node->next;

    NeonRTOS_ExitCritical(0);
}

void list_get_prev_node(tListNode *ref_node, tListNode **node)
{
    NeonRTOS_EnterCritical();

    *node = ref_node->prev;

    NeonRTOS_ExitCritical(0);
}

#ifdef __cplusplus
}
#endif