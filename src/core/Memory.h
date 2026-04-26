#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include <memory>
#include "core/Types.h"

namespace s0::memory {

// ------------------------------------------------------------------
// Arena Allocator (Linear Bump Allocator)
// Zero-allocation during frame loop. Reset at end of frame/level.
// ------------------------------------------------------------------
class Arena {
public:
    explicit Arena(size_t capacity) : m_capacity(capacity), m_offset(0) {
        m_buffer = std::make_unique<u8[]>(capacity);
    }
    
    ~Arena() = default;
    
    // Prevent copying
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
    
    // Allocate memory aligned to 'alignment'
    void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        size_t aligned_offset = (m_offset + alignment - 1) & ~(alignment - 1);
        
        if (aligned_offset + size > m_capacity) {
            S0_LOG_ERROR("Arena overflow!");
            return nullptr;
        }
        
        void* ptr = m_buffer.get() + aligned_offset;
        m_offset = aligned_offset + size;
        return ptr;
    }
    
    // Template helper for typed allocation
    template<typename T>
    T* Alloc() {
        return reinterpret_cast<T*>(Allocate(sizeof(T), alignof(T)));
    }
    
    template<typename T>
    T* AllocArray(size_t count) {
        return reinterpret_cast<T*>(Allocate(sizeof(T) * count, alignof(T)));
    }
    
    // Reset the arena (does not free memory, just resets offset)
    void Reset() {
        m_offset = 0;
    }
    
    // Get current usage
    size_t Used() const { return m_offset; }
    size_t Capacity() const { return m_capacity; }
    float UsagePercent() const { return (float)m_offset / (float)m_capacity * 100.0f; }
    
    // Get raw pointer (for GPU upload)
    const void* Data() const { return m_buffer.get(); }
    void* Data() { return m_buffer.get(); }

private:
    std::unique_ptr<u8[]> m_buffer;
    size_t m_capacity;
    size_t m_offset;
};

// ------------------------------------------------------------------
// Object Pool (Fixed-size block allocator for entities/projectiles)
// Zero fragmentation, O(1) alloc/free
// ------------------------------------------------------------------
template<typename T, size_t Capacity>
class ObjectPool {
    struct Block {
        T data;
        bool in_use;
    };
    
public:
    ObjectPool() : m_free_list_head(0) {
        for (size_t i = 0; i < Capacity - 1; ++i) {
            m_blocks[i].in_use = false;
            m_next_indices[i] = static_cast<u16>(i + 1);
        }
        m_next_indices[Capacity - 1] = 0xFFFF; // End of list
        m_count = 0;
    }
    
    T* Allocate() {
        if (m_free_list_head == 0xFFFF) {
            S0_LOG_ERROR("ObjectPool exhausted!");
            return nullptr;
        }
        
        u16 index = m_free_list_head;
        m_free_list_head = m_next_indices[index];
        
        m_blocks[index].in_use = true;
        m_count++;
        
        return &m_blocks[index].data;
    }
    
    void Free(T* ptr) {
        if (ptr == nullptr) return;
        
        // Calculate index from pointer
        uintptr_t base = reinterpret_cast<uintptr_t>(&m_blocks[0]);
        uintptr_t target = reinterpret_cast<uintptr_t>(ptr);
        size_t index = (target - base) / sizeof(Block);
        
        if (index >= Capacity || !m_blocks[index].in_use) {
            S0_LOG_ERROR("Invalid free in ObjectPool!");
            return;
        }
        
        m_blocks[index].in_use = false;
        m_next_indices[index] = m_free_list_head;
        m_free_list_head = static_cast<u16>(index);
        m_count--;
    }
    
    size_t Count() const { return m_count; }
    size_t Available() const { return Capacity - m_count; }
    
    // Iterate over active objects
    T* Begin() {
        for (size_t i = 0; i < Capacity; ++i) {
            if (m_blocks[i].in_use) {
                return &m_blocks[i].data;
            }
        }
        return nullptr;
    }
    
    T* Next(T* current) {
        uintptr_t base = reinterpret_cast<uintptr_t>(&m_blocks[0]);
        uintptr_t curr = reinterpret_cast<uintptr_t>(current);
        size_t start_index = (curr - base) / sizeof(Block) + 1;
        
        for (size_t i = start_index; i < Capacity; ++i) {
            if (m_blocks[i].in_use) {
                return &m_blocks[i].data;
            }
        }
        return nullptr;
    }

private:
    Block m_blocks[Capacity];
    u16 m_next_indices[Capacity];
    u16 m_free_list_head;
    size_t m_count;
};

// ------------------------------------------------------------------
// Stack Allocator (LIFO, for temporary scoped allocations)
// ------------------------------------------------------------------
class StackAllocator {
public:
    explicit StackAllocator(size_t capacity) : m_capacity(capacity), m_top(0) {
        m_buffer = std::make_unique<u8[]>(capacity);
    }
    
    void* Push(size_t size, size_t alignment = alignof(std::max_align_t)) {
        size_t aligned_top = (m_top + alignment - 1) & ~(alignment - 1);
        
        if (aligned_top + size > m_capacity) {
            S0_LOG_ERROR("StackAllocator overflow!");
            return nullptr;
        }
        
        void* ptr = m_buffer.get() + aligned_top;
        m_top = aligned_top + size;
        return ptr;
    }
    
    void Pop(void* ptr) {
        if (ptr == nullptr) return;
        
        uintptr_t ptr_addr = reinterpret_cast<uintptr_t>(ptr);
        uintptr_t base = reinterpret_cast<uintptr_t>(m_buffer.get());
        
        if (ptr_addr < base || ptr_addr >= base + m_top) {
            S0_LOG_WARN("Popping invalid address from StackAllocator");
            return;
        }
        
        // Only allow popping the most recent allocation
        if (ptr_addr + ((ptr_addr - base + sizeof(u8*) - 1) & ~(sizeof(u8*) - 1)) != m_top) {
            S0_LOG_WARN("StackAllocator pop order violation");
            return;
        }
        
        m_top = ptr_addr - base;
    }
    
    void Reset() { m_top = 0; }
    size_t Used() const { return m_top; }

private:
    std::unique_ptr<u8[]> m_buffer;
    size_t m_capacity;
    size_t m_top;
};

} // namespace s0::memory
