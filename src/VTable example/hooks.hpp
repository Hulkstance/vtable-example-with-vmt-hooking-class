#pragma once

/*
Credits: ReactiioN @ UnknownCheats
*/

#include <cstdint>
#include <memory>
#include <type_traits>

class vmt_hook_t
{
public:
    vmt_hook_t() = default;

    vmt_hook_t(const vmt_hook_t&) = delete;

    /// <summary>
    /// Move assignment constructor.
    /// </summary>
    /// <param name="vmthook">The VMT hook.</param>
    /// <returns></returns>
    vmt_hook_t(vmt_hook_t&& vmthook) noexcept
    {
        *this = std::move(vmthook);
    }

    ~vmt_hook_t()
    {
        shutdown();
    }

    vmt_hook_t& operator=(const vmt_hook_t&) = delete;

    /// <summary>
    /// Move assignment operator.
    /// </summary>
    /// <param name="rhs">The right hand side.</param>
    /// <returns>A shallow copy of this object.</returns>
    vmt_hook_t& operator=(vmt_hook_t&& rhs) noexcept
    {
        m_class_pointer = rhs.m_class_pointer;
        m_original_vtable_pointer = rhs.m_original_vtable_pointer;
        m_copied_vtable = std::move(rhs.m_copied_vtable);
        m_num_virtual_methods = rhs.m_num_virtual_methods;

        rhs.m_class_pointer = nullptr;
        rhs.m_original_vtable_pointer = nullptr;
        rhs.m_num_virtual_methods = 0;

        return *this;
    }

    /// <summary>
    /// Initializes the object.
    /// </summary>
    /// <param name="class_pointer">The class pointer.</param>
    /// <returns>Boolean whether it succeeds or not.</returns>
    bool initialize(void* class_pointer)
    {
        shutdown();
        
        if (class_pointer) 
        {
            m_class_pointer = reinterpret_cast<uintptr_t**>(class_pointer);
            m_original_vtable_pointer = *m_class_pointer;
            
            while (reinterpret_cast<uintptr_t*>(m_original_vtable_pointer[m_num_virtual_methods])) 
            {
                m_num_virtual_methods++;
            }
            
            if (m_num_virtual_methods > 0) 
            {
                m_copied_vtable = std::make_unique<uintptr_t[]>(m_num_virtual_methods);
                memcpy(m_copied_vtable.get(), m_original_vtable_pointer, m_num_virtual_methods * sizeof(uintptr_t));
            }
        }

        return m_copied_vtable != nullptr;
    }

    /// <summary>
    /// Replaces a methd address (hooking).
    /// </summary>
    /// <param name="index">Zero-based index.</param>
    /// <param name="hooked_function">The hooked function.</param>
    /// <returns></returns>
    uintptr_t replace_method_address(const size_t index, const uintptr_t hooked_function) const
    {
        if (m_copied_vtable && m_original_vtable_pointer && index < m_num_virtual_methods) 
        {
            m_copied_vtable[index] = hooked_function;
            return m_original_vtable_pointer[index];
        }

        return 0;
    }

    /// <summary>
    /// Gets method address at index.
    /// </summary>
    /// <param name="index">Zero-based index.</param>
    /// <returns>The method address.</returns>
    __forceinline uintptr_t get_method_address(const size_t index) const
    {
        return !m_original_vtable_pointer || index >= m_num_virtual_methods
            ? 0
            : m_original_vtable_pointer[index];
    }

    /// <summary>
    /// Hooks the class pointer.
    /// </summary>
    void hook() const
    {
        if (m_class_pointer && m_copied_vtable) 
        {
            *m_class_pointer = m_copied_vtable.get();
        }
    }
    
    /// <summary>
    /// Unhooks the class pointer.
    /// </summary>
    void unhook() const
    {
        if (m_class_pointer && m_original_vtable_pointer) 
        {
            *m_class_pointer = m_original_vtable_pointer;
        }
    }

    /// <summary>
    /// Shuts down this object and frees any resources it is using.
    /// </summary>
    void shutdown()
    {
        unhook();
        m_class_pointer = nullptr;
        m_original_vtable_pointer = nullptr;
        m_num_virtual_methods = 0;
        m_copied_vtable.reset();
    }

    /// <summary>
    /// Replaces a method address (hooking).
    /// </summary>
    /// <typeparam name="T">Generic type parameter.</typeparam>
    /// <param name="index">Zero-based index.</param>
    /// <param name="hooked_function">The hooked function address.</param>
    /// <returns>Method address at index casted as generic type parameter.</returns>
    template<typename T = uintptr_t>
    T replace_method(const size_t index, const uintptr_t hooked_function)
    {
        static_assert(
            std::is_integral<T>::value || std::is_pointer<T>::value,
            "Type T has to be a (function-)pointer or an integer"
            );

        /// If the return type would be still uintptr_t, the compiler would throw an error
        /// that it is impossible to cast an uintptr_t to an uintptr_t using reinterpret_cast.
        return reinterpret_cast<T>(reinterpret_cast<void*>(replace_method_address(index, hooked_function)));
    }

    /// <summary>
    /// Gets a method address casted as T.
    /// </summary>
    /// <typeparam name="T">Generic type parameter.</typeparam>
    /// <param name="index">Zero-based index.</param>
    /// <returns>Method address at index casted as generic type parameter.</returns>
    template<typename T = uintptr_t>
    T get_method(const size_t index) const
    {
        static_assert(
            std::is_integral<T>::value || std::is_pointer<T>::value,
            "Type T has to be a (function-)pointer or an integer"
            );

        /// If the return type would be still uintptr_t, the compiler would throw an error
        /// that it is impossible to cast an uintptr_t to an uintptr_t using reinterpret_cast.
        return reinterpret_cast<T>(reinterpret_cast<void*>(get_method_address(index)));
    }

private:
    uintptr_t** m_class_pointer = nullptr;
    uintptr_t* m_original_vtable_pointer = nullptr;
    std::unique_ptr<uintptr_t[]> m_copied_vtable = nullptr;
    size_t m_num_virtual_methods = 0;
};