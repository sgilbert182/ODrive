/*******************************************************************************
* File          : LinkedList.hpp
*
* Description   : 
*
* Project       :
*
* Author        : s.gilbert
*
* Created on    : 21 Feb 2020
*
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LINKEDLIST_HPP
#define LINKEDLIST_HPP

/******************************************************************************
INCLUDES
*******************************************************************************/

#include "stddef.h"
#include "stdint.h"
#include "MemoryHandler.hpp"
#include "string.h"

/*******************************************************************************
DEFINITIONS
*******************************************************************************/


#define LL_SUCCESS (0)
#define LL_FAIL (-1)

/*******************************************************************************
TYPES
*******************************************************************************/

/*******************************************************************************
GLOBAL VARIABLES
*******************************************************************************/

/*******************************************************************************
CONSTANTS
*******************************************************************************/

/*******************************************************************************
NAMESPACE
*******************************************************************************/

template <class MyType>
class CNode
{
using node_t = CNode<MyType>;

public:
    CNode(MyType data, node_t * pPrevious = nullptr, node_t * pNext = nullptr)
        : m_data(data)
        , m_pPrevious(pPrevious)
        , m_pNext(pNext)
    {}

public:
    MyType m_data;                                                              /* stored data */
    node_t * m_pPrevious;                                                       /* pointer to previous data node */
    node_t * m_pNext;                                                           /* pointer to next data node */
};

template <class MyType>
class CLinkedList
{
using node_t = CNode<MyType>;

public:
    CLinkedList(void * pTable = nullptr, size_t size = 0);
    ~CLinkedList() = default;
    int32_t pushToFront(MyType * pData);
    int32_t pushToBack(MyType * pData);
    int32_t pushAfter(node_t * pPrevNode, MyType * pData);
    int32_t peak(node_t ** ppRef, MyType * pData);
    int32_t countNodes(void);
    int32_t popFromFront(MyType * pData);
    int32_t popFromBack(MyType * pData);
    bool DeleteNodeAtGivenIdx(size_t idx);
    void * findNode(uint32_t nodeID);

private:
    void populateNode(MyType * pData, node_t * pCurrent, node_t * pNext, node_t * pPrevious);
    void flushNode(node_t * pNode);
    bool deleteNode(node_t * pDelete);

private:
    CMemoryManager<node_t> m_table;                                            /* pointer to the data storage table */
    size_t m_length;                                                            /* length of the list */
    node_t * m_pHead;                                                           /* pointer to the first node in the chain */
};

/*******************************************************************************
INLINE FUNCTION DEFINITIONS
*******************************************************************************/

/**\brief   Constructor
 *
 * \param   pTable  - pointer to memory space to be used to store link list
 * \param   size    - number of elements table can store
 *
 * \return  None
 */
template <class MyType>
inline CLinkedList<MyType>::CLinkedList(void * pTable, size_t size)
    : m_table((node_t *)pTable, size)
    , m_length(0)
    , m_pHead(nullptr)
{}

/**\brief   Push data element to the front of the list
 *
 * \param   ppHeadRef   - pointer to the list
 * \param   pData       - pointer to the data to store
 *
 * \return  LL_SUCCESS on LL_SUCCESSful allocation else LL_FAIL
 */
template <class MyType>
inline int32_t CLinkedList<MyType>::pushToFront(MyType * pData)
{
    int32_t returnVal = LL_FAIL;
    node_t * newNode = m_table.getBuffer();

    if (nullptr != newNode)
    {
        /* copy in the data to the new node */
        /* Make 'next' of newNode point to headRef and 'previous' nullptr */
        populateNode(pData, newNode, m_pHead, nullptr);

        if (nullptr != m_pHead)                                                 /* change 'previous' of head node to new node */
        {
            m_pHead->m_pPrevious = newNode;
        }

        m_pHead = newNode;                                                      /* move the head to point to the new node */
        returnVal = LL_SUCCESS;
    }

    return returnVal;
}

/**\brief   Push data element to the back of the list
 *
 * \param   pData   - pointer to the data to store
 *
 * \return  LL_SUCCESS on LL_SUCCESSful allocation else LL_FAIL
 */
template <class MyType>
inline int32_t CLinkedList<MyType>::pushToBack(MyType * pData)
{
    int32_t returnVal = LL_FAIL;
    node_t * newNode = m_table.getBuffer();                                   /* allocate node */

    if (nullptr != newNode)
    {
        node_t * last = (node_t *)findLast();
        last->m_pNext = newNode;                                                /* Change the next of last node */

        /* copy in the data  */
        /* This new node is going to be the last node, so make pointer to next nullptr */
        /* Make last node as previous of new node */
        populateNode(pData, newNode, nullptr, last);

        returnVal = LL_SUCCESS;
    }

    return returnVal;
}

/**\brief   Insert data element after selected node.
 *
 * \param   pPrevNode   - pointer to node to insert after
 * \param   pData       - pointer to the data to store
 *
 * \return  LL_SUCCESS on LL_SUCCESSful allocation else LL_FAILED
 */
template <class MyType>
inline int32_t CLinkedList<MyType>::pushAfter(node_t * pPrevNode, MyType * pData)
{
    int32_t returnVal = LL_FAIL;
    node_t * newNode = m_table.getBuffer();                                   /* allocate node */

    if (nullptr != pPrevNode)                                                   /* check if the given prev_node is NULL */
    {
        /* copy in the data to the new node */
        /* Make next of new node as next of prev_node */
        /* Make prev_node as previous of newNode */
        populateNode(pData, newNode, pPrevNode->m_pNext, pPrevNode);

        pPrevNode->m_pNext = newNode;                                           /* Make the next of prev_node as newNode */

        if (nullptr != newNode->m_pNext)                                        /* Change previous of newNode's next node */
        {
            newNode->m_pNext->m_pPrevious = newNode;
        }
        returnVal = LL_SUCCESS;
    }

    return returnVal;
}

/**\brief   Peaks at element
 *
 * \param   ppRef       - pointer to the list
 * \param   pData       - pointer to where to write the data to
 *
 * \return  LL_SUCCESS on LL_SUCCESSful allocation else LL_FAIL
 */
template <class MyType>
 inline int32_t CLinkedList<MyType>::peak(node_t ** ppRef, MyType * pData)
{
    int32_t returnVal = LL_FAIL;
    node_t * pNode = *ppRef;

    if (nullptr != pNode)
    {
        memcpy(pData, (void *)&pNode->m_data, sizeof(MyType));                  /* copy out the data to the target pointer */
        returnVal = LL_SUCCESS;
    }

    return returnVal;
}

/**\brief   Counts the number of elements in the list
 *
 * \param   None
 *
 * \return  the number of nodes
 */
template <class MyType>
inline int32_t CLinkedList<MyType>::countNodes(void)
{
    int32_t count = 0;
    node_t * pLast = m_pHead;

    if (nullptr != pLast)
    {
        count = 1;
        while (nullptr != pLast->m_pNext)                                       /* Traverse to the last node */
        {
            ++count;
            pLast = pLast->m_pNext;
        }
    }

    return count;
}

/**\brief   Pop first element off the list
 *
 * \param   pData           - pointer to where to write the data to
 *
 * \return  LL_SUCCESS on LL_SUCCESSful allocation else LL_FAIL
 */
template <class MyType>
inline int32_t CLinkedList<MyType>::popFromFront(MyType * pData)
{
    int32_t returnVal = LL_FAIL;

    if (LL_SUCCESS == Peak(m_pHead, pData))
    {
        node_t * pToDelete = m_pHead;

        returnVal = deleteNode(pToDelete);

        flushNode(pToDelete);
        m_table.releaseBuffer(pToDelete);
        returnVal = LL_SUCCESS;
    }

    return returnVal;
}

/**\brief   Pop last element off the list
 *
 * \param   pData   - pointer to where to write the data to
 *
 * \return  LL_SUCCESS on LL_SUCCESSful allocation else LL_FAIL
 */
template <class MyType>
inline int32_t CLinkedList<MyType>::popFromBack(MyType * pData)
{
    int32_t returnVal = LL_FAIL;
    node_t * pLast = (node_t *)findLast();

    if (nullptr != pLast)
    {
        if (LL_SUCCESS == Peak(pLast, pData))
        {
            returnVal = deleteNode(pLast);
        }
    }

    return returnVal;
}

/**\brief   Deletes node at given index position from head of list
 * From https://www.geeksforgeeks.org/delete-doubly-linked-list-node-given-position/
 *
 * \param   idx             - index
 *
 * \return  LL_SUCCESS on LL_SUCCESSful allocation else LL_FAIL
 */
template <class MyType>
inline bool CLinkedList<MyType>::DeleteNodeAtGivenIdx(size_t idx)
{
    int32_t returnVal = LL_FAIL;

    /* if list in NULL or invalid position is given */
    if ((nullptr != *m_pHead) && ((int32_t)idx <= (countNodes() - 1)))
    {
        node_t * pDelete = m_pHead;

        /* traverse up to the node to delete at position 'idx' from head */
        for (size_t i = 0; i < idx; ++i)
        {
            pDelete = pDelete->m_pNext;
        }

        /* delete the node pointed to by 'current' */
        returnVal = deleteNode(pDelete);
    }

    return returnVal;
}

/**\brief   Deletes node in list and corrects the nodes either side to maintain
 *          the linked list chain.
 *
 * \param   pDelete - pointer to node to delete
 *
 * \return  bool
 */
template <class MyType>
inline bool CLinkedList<MyType>::deleteNode(node_t * pDelete)
{
    int32_t returnVal = LL_FAIL;
    /* base case */
    if ((nullptr != m_pHead) && (nullptr != pDelete))
    {

        /* If node to be deleted is head node */
        if (m_pHead == pDelete)
        {
            m_pHead = pDelete->m_pNext;
        }

        /* Change next only if node to be deleted is NOT the last node */
        if (nullptr != pDelete->m_pNext)
        {
            pDelete->m_pNext->m_pPrevious = pDelete->m_pPrevious;
        }

        /* Change prev only if node to be deleted is NOT the first node */
        if (nullptr != pDelete->m_pPrevious)
        {
            pDelete->m_pPrevious->m_pNext = pDelete->m_pNext;
        }

        flushNode(pDelete);
        m_table.releaseBuffer(pDelete);
        returnVal = LL_SUCCESS;
    }
    return returnVal;
}

/**\brief   Populates current node with the data, and next and previous pointers
 *
 * \param   pData       - pointer to the data to store
 * \param   pCurrent    - pointer to current node
 * \param   pNext       - pointer to next node
 * \param   pPrevious   - pointer to previous node
 *
 * \return  None
 */
template <class MyType>
void CLinkedList<MyType>::populateNode(MyType * pData, node_t * pCurrent, node_t * pNext, node_t * pPrevious)
{
    if(nullptr != pCurrent)
    {
        memcpy(&pCurrent->m_data, pData, sizeof(MyType));                       /* copy in the data to the current node */
        pCurrent->m_pNext = pNext;                                              /* Make 'next node' pointer of the current node point to the previous node's 'next node' */
        pCurrent->m_pPrevious = pPrevious;                                      /* Make 'previous node' pointer of the current node point to the previous node */
    }
}

/**\brief   Erases data in the node and resets the pointers
 *
 * \param   pNode   - pointer to current node
 *
 * \return  None
 */
template <class MyType>
void CLinkedList<MyType>::flushNode(node_t * pNode)
{
    if(nullptr != pNode)
    {
        MyType tempData;
        populateNode(memset(tempData, 0, sizeof(MyType)), pNode, nullptr, nullptr);
    }
}

/**\brief   Finds last active node and returns address
 *
 * \param   None
 *
 * \return  pointer to last node
 */
template <class MyType>
inline void * CLinkedList<MyType>::findLast(void)
{
    node_t * pLast = m_pHead;

    while (nullptr != pLast->m_pNext)                                           /* Traverse to the last node */
    {
        pLast = pLast->m_pNext;
    }

    return (void *)pLast;
}

#endif /* LINKEDLIST_HPP */
