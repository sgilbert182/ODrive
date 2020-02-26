/*******************************************************************************
* File          : MemoryHandler.hpp
*
* Description   : Pseudo dynamic memory manager, uses statically allocated
*                   memory and dynamically assigns and desassigns segments.
*
* Project       :
*
* Author        : s.gilbert
*
* Created on    : 21 Feb 2020
*
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MEMORYHANDLER_HPP
#define MEMORYHANDLER_HPP

/******************************************************************************
INCLUDES
*******************************************************************************/

#include "stdint.h"
#include "stddef.h"

/*******************************************************************************
DEFINITIONS
*******************************************************************************/

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

template <class storageType>
class CMemoryTracker
{
public:
    CMemoryTracker(void * pTable, size_t size)
    : m_pTable((storageType *)pTable)
    , m_size(size)
{}
    ~CMemoryTracker() = default;

private:
    storageType * m_pTable;                                                     /* pool of memory */
    size_t m_size;
};

template <class storageType>
class CMemoryManager
{
public:
    CMemoryManager(void * pMemory, size_t size);
    ~CMemoryManager() = default;
    storageType * getBuffer(void);
    void releaseBuffer(storageType const * const pBuff);
    size_t getMaxSlotCount(void);

private:
    typedef struct
    {
        storageType data;
        bool inUse;
    }slotDetails_t;

    slotDetails_t * m_pTable;
    size_t m_slotSize;                                                          /* byte count of each element */
    uint32_t m_maxSlots;                                                        /* number of individual data segments that can be stored */
};

/*******************************************************************************
INLINE FUNCTION DEFINITIONS
*******************************************************************************/

/**\brief   Constructor.
 *
 * \param   pMemory - pointer to memory to dynamic allocate
 * \param   size    - byte count of allocated memory
 *
 * \return  None
 */
template <class storageType>
CMemoryManager<storageType>::CMemoryManager(void * pMemory, size_t size)
    : m_pTable((slotDetails_t *)pMemory)
    , m_slotSize(sizeof(slotDetails_t))                                         /* byte count of each element */
    , m_maxSlots(size / sizeof(slotDetails_t))                                  /* number of individual data segments that can be stored */
{};

/**\brief   Get a pointer to the memory slot
 *
 * \param   None
 *
 * \return  Returns a pointer to the requested buffer,
 *          if available, else it returns NULL
 */
template <class storageType>
storageType * CMemoryManager<storageType>::getBuffer(void)
{
    storageType * returnVal = nullptr;

    for (auto slotIndex = 0u; slotIndex < m_maxSlots; ++slotIndex)
    {
        slotDetails_t * pSlotDetails = &m_pTable[slotIndex];
        if (!pSlotDetails->inUse)
        {
            pSlotDetails->inUse = true;
            returnVal = &pSlotDetails->data;
            break;
        }
    }

    return returnVal;
}

/**\brief   Release the specified buffer once its no longer needed. This
 *          function checks the pointer address matches the  viable addresses.
 *
 * \param   pBuff   - pointer to the buffer
 *
 * \return  None
 */
template <class storageType>
void CMemoryManager<storageType>::releaseBuffer(storageType const * const pBuff)
{
    for (auto slotIndex = 0u; slotIndex < m_maxSlots; ++slotIndex)
    {
        slotDetails_t * slotDetailsPtr = &m_pTable[slotIndex];

        if (pBuff == &slotDetailsPtr->data)                                     /* if slot pointer matches buffer pointer */
        {
            slotDetailsPtr->inUse = false;                                      /* clear usage flag */
            break;
        }
    }
}

/**\brief   Returns maximum number of slots that can be allocated
 *
 * \param   None
 *
 * \return  max number of slots
 */
template <class storageType>
size_t CMemoryManager<storageType>::getMaxSlotCount(void)
{
    return m_maxSlots;
}

#endif /* MEMORYHANDLER_HPP */
