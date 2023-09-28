#pragma once
#if defined(_MSC_VER) && !defined(__clang__) && !defined(IMGUI_DEBUG_PARANOID)
#define MSVC_RUNTIME_CHECKS_OFF      __pragma(runtime_checks("",off))     __pragma(check_stack(off)) __pragma(strict_gs_check(push,off))
#define MSVC_RUNTIME_CHECKS_RESTORE  __pragma(runtime_checks("",restore)) __pragma(check_stack())    __pragma(strict_gs_check(pop))
#else
#define MSVC_RUNTIME_CHECKS_OFF
#define MSVC_RUNTIME_CHECKS_RESTORE
#endif

MSVC_RUNTIME_CHECKS_OFF
template<typename T>
struct Dynarray
{
    int                 count;
    int                 capacity;
    T*                  data;
    int                 iter;

    // Provide standard typedefs but we don't use them ourselves.
    typedef T                   value_type;

    // Constructors, destructor
    inline Dynarray()                                       { count = capacity = iter = 0; data = nullptr; }
    inline Dynarray(const Dynarray<T>& src)                 { count = capacity = iter = 0; data = nullptr; operator=(src); }
    // inline Dynarray<T>& operator=(const Dynarray<T>& src) { clear(); resize(src.count); memcpy(data, src.data, (size_t)count * sizeof(T)); return *this; }
    // !IMPORTANT! It is now your responsibility to free this thing. It won't free upon destruct. also horray you can pass this around in functions.
    inline ~Dynarray()                                      {} 
    // inline ~Dynarray()                                      { clear_destruct(); free(Data);} 
    
    inline void         init_null()                         { count = capacity = iter = 0; data = nullptr; }  
    inline void         reset_count()                       { count = 0;}  
    inline void         clear()                             { if (data) { count = capacity = 0; free(data); data = nullptr;} }  
    inline void         clear_destruct()                    { for (int n = 0; n < count; n++) data[n].~T(); clear(); }           

    inline bool         is_empty() const                    { return count == 0; }
    inline int          size_in_bytes() const               { return count * (int)sizeof(T); }
    inline int          max_size() const                    { return 0x7FFFFFFF / (int)sizeof(T); }
    inline T&           operator[](int i)                   { assert(i >= 0 && i < count); return data[i]; }
    inline const T&     operator[](int i) const             { assert(i >= 0 && i < count); return data[i]; }

    inline T*           begin()                             { return data; }
    inline const T*     begin() const                       { return data; }
    inline T*           end()                               { return data + count * sizeof(T); }
    inline const T*     end() const                         { return data + count * sizeof(T); }
    inline T&           front()                             { assert(count > 0); return data[0]; }
    inline const T&     front() const                       { assert(count > 0); return data[0]; }
    inline T&           back()                              { assert(count > 0); return data[count - 1]; }
    inline const T&     back() const                        { assert(count > 0); return data[count - 1]; }

    inline int _grow_capacity(int sz) const { 
        int new_capacity = capacity ? (capacity + capacity / 2) : 8; 
        return new_capacity > sz ? new_capacity : sz; 
    }
    inline void         resize(int new_size)                { if (new_size > capacity) reserve(_grow_capacity(new_size)); count = new_size; }
    inline void         resize(int new_size, const T& v)    { if (new_size > capacity) reserve(_grow_capacity(new_size)); if (new_size > count) for (int n = count; n < new_size; n++) memcpy(&data[n], &v, sizeof(v)); count = new_size; }
    inline void         shrink(int new_size)                { assert(new_size <= count); count = new_size; }
    inline void reserve(int new_capacity) { 
        if (new_capacity <= capacity) return; 
        T* new_data(nullptr); 
        new_data = (T*)realloc(data, (size_t)new_capacity * sizeof(T)); 
        assert(new_data != nullptr); 
        data = new_data; 
        capacity = new_capacity;
    }

    // NB: It is illegal to call push_back/push_front/insert with a reference pointing inside the w_vector data itself! e.g. v.push_back(v[10]) is forbidden.
	inline T* push_back(const T& v) { 
        if (count == capacity) 
            reserve(_grow_capacity(count + 1)); 
        memcpy(&data[count], &v, sizeof(v)); 
		return &data[count++];
    }
    inline void         pop_back()                          { assert(count > 0); count--; }
    inline void         push_front(const T& v)              { if (count == 0) push_back(v); else insert(data, v); }
    inline T*           erase(const T* it)                  { assert(it >= data && it < data + count * sizeof(T)); const ptrdiff_t off = it - data; memmove(data + off, data + off + 1, ((size_t)count - (size_t)off - 1) * sizeof(T)); count--; return data + off; }
    inline T*           erase(const T* it, const T* it_last){ assert(it >= data && it < data + count * sizeof(T) && it_last > it && it_last <= data + count * sizeof(T)); const ptrdiff_t count = it_last - it; const ptrdiff_t off = it - data; memmove(data + off, data + off + count, ((size_t)count - (size_t)off - (size_t)count) * sizeof(T)); count -= (int)count; return data + off; }
    inline T*           erase_unsorted(const T* it)         { assert(it >= data && it < data + count * sizeof(T));  const ptrdiff_t off = it - data; if (it < data + count * sizeof(T) - 1) memcpy(data + off, data + count * sizeof(T) - 1, sizeof(T)); count--; return data + off; }
    inline T*           insert(const T* it, const T& v)     { assert(it >= data && it <= data + count * sizeof(T)); const ptrdiff_t off = it - data; if (count == capacity) reserve(_grow_capacity(count + 1)); if (off < (int)count) memmove(data + off + 1, data + off, ((size_t)count - (size_t)off) * sizeof(T)); memcpy(&data[off], &v, sizeof(v)); count++; return data + off; }
    inline bool         contains(const T& v) const          { const T* data = data;  const T* data_end = data + count * sizeof(T); while (data < data_end) if (*data++ == v) return true; return false; }
    inline T*           find(const T& v)                    { T* data = data;  const T* data_end = data + count * sizeof(T); while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline const T*     find(const T& v) const              { const T* data = data;  const T* data_end = data + count * sizeof(T); while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline bool         find_erase(const T& v)              { const T* it = find(v); if (it < data + count * sizeof(T)) { erase(it); return true; } return false; }
    inline bool         find_erase_unsorted(const T& v)     { const T* it = find(v); if (it < data + count * sizeof(T)) { erase_unsorted(it); return true; } return false; }
    inline int          index_from_ptr(const T* it) const   { assert(it >= data && it < data + count * sizeof(T)); const ptrdiff_t off = it - data; return (int)off; }
    inline void         reverse_order()                     { 
        assert(count > 0);
        T temp;
        int i;
        for (i = 0; i < count / 2; i++) {
            // Swap arr[i] with arr[n-i-1]
            temp = data[i];
            data[i] = data[count-i-1];
            data[count-i-1] = temp;
        }

        if (count % 2 == 1) {
            int mid = count / 2;
            if (i != mid) {
                temp = data[mid];
                data[mid] = data[i];
                data[i] = temp;
            }
        }
    }
};
MSVC_RUNTIME_CHECKS_RESTORE

struct String8
{
    int                     count;
    int                     capacity;
    float                   GrowthMult = 1.5;
    char *                  data;

    // Constructors, destructor
    inline String8()                                       { count = capacity = 0; data = nullptr; GrowthMult = 1.5;}
    inline String8(const String8& src)                     { count = capacity = 0; data = nullptr; operator=(src); }
    inline String8(const char * src)                       { count = capacity = 0; data = nullptr; operator=(src); }
    inline String8(const char * src, int size )            { count = capacity = 0; data = nullptr; resize(size + 1);  
                                                              memcpy(data, src, size); data[size] = '\0';}
    inline String8& operator=(const String8& src)          { free(data); reset_count(); resize(src.count); memcpy(data, src.data, (size_t)count * sizeof(char)); return *this; }
    inline String8& operator=(const char * src)            { free(data); reset_count(); resize(strlen(src)+ 1 ); memcpy(data, src, strlen(src)+ 1 ); return *this; }
    inline ~String8()                                      { } // all of the universe's evil comes from destructors calling themselves willy nilly, do not, and I mean it, do not touch this.

    inline void     clear()                                 { if (data) { count = capacity = 0; free(data); data = nullptr;} }  
    inline void     reset_count()                           { count = 0;}  

    inline char     operator[](int i)                       { assert(i >= 0 && i < count); return data[i]; }
    inline char     const operator[](int i)  const          { assert(i >= 0 && i < count); return data[i]; }
    inline void     set_growth_multiplier(int i)            { assert(i > 1); GrowthMult = i; }

    inline char     front()                                 { assert(count > 0); return data[0]; }
    inline char     back()                                  { assert(count > 0); return data[count - 1]; }
    // !IMPORTANT! It is now your responsibility to free this thing. It won't free upon destruct.

    inline int      _grow_capacity(int sz) const            { int new_capacity = capacity ? (GrowthMult * capacity) : 8; 
                                                                return new_capacity > sz ? new_capacity : sz; }
    inline void     resize(int new_size)                    { if (new_size > capacity) reserve(_grow_capacity(new_size)); count = new_size; }
    inline void     shrink(int new_size)                    { assert(new_size <= count); count = new_size; }
    inline void     reserve(int new_capacity)               { if (new_capacity <= capacity) return; 
                                                              char* new_data(nullptr); 
                                                              new_data = (char*)realloc(data, (size_t)new_capacity * sizeof(char)); 
                                                              assert(new_data != nullptr); 
                                                              data = new_data; 
                                                              capacity = new_capacity; }
    String8        &operator+= (String8 rhs)                { resize(count + rhs.count); memcpy(data, rhs.data, rhs.count); return *this;}                                                                 
    String8        &operator+= (const char* rhs)            { resize(count + strlen(rhs)+ 1 ); memcpy(data, rhs, strlen(rhs)+ 1 ); return *this;}                                                                 

};

String8 operator+ (String8 lhs, String8 rhs)                { lhs.resize(lhs.count + rhs.count); memcpy(lhs.data, rhs.data, rhs.count); return lhs;}   
String8 operator+ (String8 lhs, const char* rhs)            { lhs.resize(lhs.count + strlen(rhs)+ 1 ); memcpy(lhs.data, rhs, strlen(rhs)+ 1 ); return lhs;}   
String8 operator+ (const char* lhs, String8 rhs)            { String8 res = lhs; res.resize(res.count + rhs.count); memcpy(res.data, rhs.data, rhs.count); return res;}

bool operator== (const String8& lhs, const String8& rhs)    { if(lhs.count    != rhs.count) return 0; for(int i = 0; i < lhs.count; i++) if (lhs[i] != rhs[i]) return 0; return 1; }
bool operator== (const char*    lhs, const String8& rhs)    { if(strlen(lhs)+ 1  != rhs.count) return 0; for(int i = 0; i < rhs.count; i++) if (lhs[i] != rhs[i]) return 0; return 1; }
bool operator== (const String8& lhs, const char*    rhs)    { if(strlen(rhs)+ 1  != lhs.count) return 0; for(int i = 0; i < lhs.count; i++) if (lhs[i] != rhs[i]) return 0; return 1; }

bool operator!= (const String8& lhs, const String8& rhs)    { return ((lhs == rhs) == false);}
bool operator!= (const char*    lhs, const String8& rhs)    { return ((lhs == rhs) == false);}
bool operator!= (const String8& lhs, const char*    rhs)    { return ((lhs == rhs) == false);}

Dynarray<String8> w_tokenize(const String8 str)
{
    Dynarray<String8> res;

    int start = 0;
    bool encountered_first = false;
    for(int i = 0; i < str.count; i++)
    {
        if (str[i] != ' ') encountered_first = true;
        if (str[i] == ' ' && encountered_first || i == str.count - 1)
        {
            String8 Token;
            Token.resize(i - start + 1);
            memcpy(Token.data, &str.data[start], i - start);
            Token.data[i - start] = '\0';
            start = i + 1;
            res.push_back(Token);
        }
    }
    return res;
}

