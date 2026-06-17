#ifndef DATA_SOURCE_H
#define DATA_SOURCE_H

int data_source_init(void);
int data_source_read(char*buf,int buf_size);
void data_source_cleanup(void);

#endif
