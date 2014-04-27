//TINDEX table_add_ptr(U8* ptr,HINDEX h);
U8** table_base(U8* p);


TOKEN table_find_or_create(TOKEN* address,TOKEN* target);
PTOKEN* table_end(PTOKEN address);
void table_wipe(PTOKEN* address);
TOKEN** table_base(TOKEN* p);
