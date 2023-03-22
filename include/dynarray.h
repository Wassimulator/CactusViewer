#if defined(_MSC_VER) && !defined(__clang__) && !defined(IMGUI_DEBUG_PARANOID)
#define MSVC_RUNTIME_CHECKS_OFF      __pragma(runtime_checks("",off))     __pragma(check_stack(off)) __pragma(strict_gs_check(push,off))
#define MSVC_RUNTIME_CHECKS_RESTORE  __pragma(runtime_checks("",restore)) __pragma(check_stack())    __pragma(strict_gs_check(pop))
#else
#define MSVC_RUNTIME_CHECKS_OFF
#define MSVC_RUNTIME_CHECKS_RESTORE
#endif

MSVC_RUNTIME_CHECKS_OFF
template<typename T>
struct dynarray
{
    int                 Count;
    int                 Capacity;
    T*                  Data;
    int                 iter;

    typedef T                   value_type;

    inline dynarray()                                       { Count = Capacity = iter = 0; Data = nullptr; }
    inline dynarray(const dynarray<T>& src)                 { Count = Capacity = iter = 0; Data = nullptr; operator=(src); }
    inline dynarray<T>& operator=(const dynarray<T>& src)   { clear(); resize(src.Count); memcpy(Data, src.Data, (size_t)Count * sizeof(T)); return *this; }
    inline ~dynarray()                                      { /*nothing happens here, so bear in mind when to free*/ } 
    
    inline void         reset_count()                       { Count = 0;}  
    inline void         clear()                             { if (Data) { Count = Capacity = 0; free(Data); Data = nullptr;} }  
    inline void         clear_destruct()                    { for (int n = 0; n < Count; n++) Data[n].~T(); clear(); }           

    inline bool         is_empty() const                    { return Count == 0; }
    inline int          size_in_bytes() const               { return Count * (int)sizeof(T); }
    inline int          max_size() const                    { return 0x7FFFFFFF / (int)sizeof(T); }
    inline int          capacity() const                    { return Capacity; }
    inline T&           operator[](int i)                   { assert(i >= 0 && i < Count); return Data[i]; }
    inline const T&     operator[](int i) const             { assert(i >= 0 && i < Count); return Data[i]; }

    inline T*           begin()                             { return Data; }
    inline const T*     begin() const                       { return Data; }
    inline T*           end()                               { return Data + Count; }
    inline const T*     end() const                         { return Data + Count; }
    inline T&           front()                             { assert(Count > 0); return Data[0]; }
    inline const T&     front() const                       { assert(Count > 0); return Data[0]; }
    inline T&           back()                              { assert(Count > 0); return Data[Count - 1]; }
    inline const T&     back() const                        { assert(Count > 0); return Data[Count - 1]; }
    inline void         swap(dynarray<T>& rhs)              { int rhs_size = rhs.Count; rhs.Count = Count; Count = rhs_size; int rhs_cap = rhs.Capacity; rhs.Capacity = Capacity; Capacity = rhs_cap; T* rhs_data = rhs.Data; rhs.Data = Data; Data = rhs_data; }

    inline int          _grow_capacity(int sz) const        { int new_capacity = Capacity ? (Capacity + Capacity / 2) : 8; 
                                                                return new_capacity > sz ? new_capacity : sz; }
    inline void         resize(int new_size)                { if (new_size > Capacity) reserve(_grow_capacity(new_size)); Count = new_size; }
    inline void         resize(int new_size, const T& v)    { if (new_size > Capacity) reserve(_grow_capacity(new_size)); if (new_size > Count) for (int n = Count; n < new_size; n++) memcpy(&Data[n], &v, sizeof(v)); Count = new_size; }
    inline void         shrink(int new_size)                { assert(new_size <= Count); Count = new_size; }
    inline void         reserve(int new_capacity)           { if (new_capacity <= Capacity) return; 
                                                              T* new_data(nullptr); 
                                                              new_data = (T*)realloc(Data, (size_t)new_capacity * sizeof(T)); 
                                                              assert(new_data != nullptr); 
                                                              Data = new_data; 
                                                              Capacity = new_capacity; }

    inline void         push_back(const T& v)               { if (Count == Capacity) reserve(_grow_capacity(Count + 1)); 
                                                                memcpy(&Data[Count], &v, sizeof(v)); Count++; }
    inline void         pop_back()                          { assert(Count > 0); Count--; }
    inline void         push_front(const T& v)              { if (Count == 0) push_back(v); else insert(Data, v); }
    inline T*           erase(const T* it)                  { assert(it >= Data && it < Data + Count); const ptrdiff_t off = it - Data; memmove(Data + off, Data + off + 1, ((size_t)Count - (size_t)off - 1) * sizeof(T)); Count--; return Data + off; }
    inline T*           erase(const T* it, const T* it_last){ assert(it >= Data && it < Data + Count && it_last > it && it_last <= Data + Count); const ptrdiff_t count = it_last - it; const ptrdiff_t off = it - Data; memmove(Data + off, Data + off + count, ((size_t)Count - (size_t)off - (size_t)count) * sizeof(T)); Count -= (int)count; return Data + off; }
    inline T*           erase_unsorted(const T* it)         { assert(it >= Data && it < Data + Count);  const ptrdiff_t off = it - Data; if (it < Data + Count - 1) memcpy(Data + off, Data + Count - 1, sizeof(T)); Count--; return Data + off; }
    inline T*           insert(const T* it, const T& v)     { assert(it >= Data && it <= Data + Count); const ptrdiff_t off = it - Data; if (Count == Capacity) reserve(_grow_capacity(Count + 1)); if (off < (int)Count) memmove(Data + off + 1, Data + off, ((size_t)Count - (size_t)off) * sizeof(T)); memcpy(&Data[off], &v, sizeof(v)); Count++; return Data + off; }
    inline bool         contains(const T& v) const          { const T* data = Data;  const T* data_end = Data + Count; while (data < data_end) if (*data++ == v) return true; return false; }
    inline T*           find(const T& v)                    { T* data = Data;  const T* data_end = Data + Count; while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline const T*     find(const T& v) const              { const T* data = Data;  const T* data_end = Data + Count; while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline bool         find_erase(const T& v)              { const T* it = find(v); if (it < Data + Count) { erase(it); return true; } return false; }
    inline bool         find_erase_unsorted(const T& v)     { const T* it = find(v); if (it < Data + Count) { erase_unsorted(it); return true; } return false; }
    inline int          index_from_ptr(const T* it) const   { assert(it >= Data && it < Data + Count); const ptrdiff_t off = it - Data; return (int)off; }
};
MSVC_RUNTIME_CHECKS_RESTORE