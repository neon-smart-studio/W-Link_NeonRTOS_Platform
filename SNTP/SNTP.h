
#ifndef SNTP_H
#define SNTP_H

#define SNTP_UPDATE_INTERVAL        30*1000

int SNTP_Init(void);
bool Is_SNTP_Has_Updated_Time();

#endif //SNTP_H