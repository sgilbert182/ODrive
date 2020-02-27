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
#include "string.h"

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
class CMemoryManager
{
public:
    CMemoryManager(void * pMemory, size_t size);
    ~CMemoryManager() = default;
    storageType * getBuffer(void);
    void releaseBuffer(storageType * const pBuff);
    size_t getMaxSlots(void);
    size_t getSlotSize(void);

private:
    void * m_pTable;
    size_t m_slotSize;                                                          /* byte count of each element */
    uint32_t m_maxSlots;                                                        /* number of individual data segments that can be stored */
    storageType * m_data;
    bool * m_inUse;
};

/*******************************************************************************
INLINE FUNCTION DEFINITIONS
*******************************************************************************/

/**\brief   Constructor.
 *
 * \param   pMemory - pointer to memory to dynamic allocate
 * \param   size    - byte count of allocated memory.
 *
 * \return  None
 */
template <class storageType>
CMemoryManager<storageType>::CMemoryManager(void * pMemory, size_t size)
    : m_pTable(pMemory)
    , m_slotSize(sizeof(storageType))                                           /* byte count of each element */
    , m_maxSlots(size / (m_slotSize + sizeof(bool)))                            /* number of individual data segments that can be stored */
    , m_data((storageType *)m_pTable)
    , m_inUse((bool *)&m_data[m_maxSlots])
{
    memset(this->m_pTable, 0, size);
};

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
        if(true != m_inUse[slotIndex])
        {
            m_inUse[slotIndex] = true;
            returnVal = &m_data[slotIndex];
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
void CMemoryManager<storageType>::releaseBuffer(storageType * const pBuff)
{
    for (auto slotIndex = 0u; slotIndex < m_maxSlots; ++slotIndex)
    {
        if(pBuff == &m_data[slotIndex])
        {
            m_inUse[slotIndex] = false;
            memset(pBuff, 0, m_slotSize);
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
size_t CMemoryManager<storageType>::getMaxSlots(void)
{
    return m_maxSlots;
}

/**\brief   Returns byte count for a single slot
 *
 * \param   None
 *
 * \return  byte count
 */
template <class storageType>
size_t CMemoryManager<storageType>::getSlotSize(void)
{
    return m_slotSize;
}

#endif /* MEMORYHANDLER_HPP */
