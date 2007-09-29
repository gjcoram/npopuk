/* nPOPuk 
 * ppcpoom.h
 * Code originally from Matthew R. Pattman (?)
 * Info at http://www.npopsupport.org.uk
 */
#ifdef __cplusplus
extern "C"{
#endif 

int UpdateAddressBook(unsigned short * szFileName, int NameOrder, int NameIsComment);
int AddPOOMContact(unsigned short *email, unsigned short *fname, unsigned short *lname);
void FormatOutputString(char *ret, WCHAR *name, WCHAR *email, WCHAR *ctgy, int fmt);

#ifdef __cplusplus
}
#endif

#if _WIN32_WCE <= 200
#define _WCE_OLD				1
#endif

#define RELEASE_OBJ(s)  \
    if (s != NULL)      \
    {                   \
        s->Release();   \
        s = NULL;       \
    }
