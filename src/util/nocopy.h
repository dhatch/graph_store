#define DISALLOW_COPY(TypeName) \
    TypeName(const TypeName&) = delete; \
    void operator=(const TypeName&) = delete;
