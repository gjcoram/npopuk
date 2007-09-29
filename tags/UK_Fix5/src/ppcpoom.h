#ifdef __cplusplus
extern "C"{
#endif 

void UpdateAddressBook(unsigned short * szFileName);

#ifdef __cplusplus
}
#endif

#define RELEASE_OBJ(s)  \
    if (s != NULL)      \
    {                   \
        s->Release();   \
        s = NULL;       \
    }
