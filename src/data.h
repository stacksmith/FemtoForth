void data_align4(void);
U8* data_compile_U8(U8 val);
U8* data_compile_U16(U16 val);
U8* data_compile_U32(U32 val);
U8* data_compile_blob(U8*blob,U32 cnt);
int data_compile_token(HINDEX h);

U8* data_compile_from_file(FILE* f,U32 cnt);
// compile a ref-style <token><ref> sequence.
int data_ref_style(HINDEX htarget,char* abspath);
int data_ref_style_p(HINDEX htarget,HINDEX hoperation);
