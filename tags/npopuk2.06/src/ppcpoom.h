/* nPOPuk 
 * ppcpoom.h
 * Code originally from Matthew R. Pattman (?)
 * Info at http://www.npopsupport.org.uk
 */
#ifdef __cplusplus
extern "C"{
#endif 

void UpdateAddressBook(unsigned short * szFileName, int NameOrder, int NameIsComment);

#ifdef __cplusplus
}
#endif

#define RELEASE_OBJ(s)  \
    if (s != NULL)      \
    {                   \
        s->Release();   \
        s = NULL;       \
    }
