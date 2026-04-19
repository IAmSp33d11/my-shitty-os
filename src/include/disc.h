#ifndef DISC_H
#define DISC_H

bool detect_discs(void);
int atapi_read(uint32_t lba, uint32_t sectors, uint16_t* buffer);
#endif