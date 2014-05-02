//
// Source
//
void src_init();
void src_ws();
U32 src_cnt();
U32 src_one();
void src_error(char* msg);
int src_file(char* fname);
void src_skip_line();
char* src_word(U32* pcnt);