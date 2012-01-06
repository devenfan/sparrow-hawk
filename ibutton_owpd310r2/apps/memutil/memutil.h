#ifndef MEMUTIL_H_INCLUDED
#define MEMUTIL_H_INCLUDED



// Defines for the menu lists
#define MAIN_MENU          0
#define BANK_MENU          1
#define ENTRY_MENU         2
// Defines for the switch statements for Main Menu
#define MAIN_SELECT_DEVICE 0
#define MAIN_QUIT          1
// Defines for the switch statements for Memory Bank
#define BANK_INFO          0
#define BANK_READ_BLOCK    1
#define BANK_READ_PAGE     2
#define BANK_READ_UDP      3
#define BANK_WRITE_BLOCK   4
#define BANK_WRITE_UDP     5
#define BANK_NEW_BANK      6
#define BANK_MAIN_MENU     7
#define BANK_BM_READ_PASS  8
#define BANK_BM_RW_PASS    9
#define BANK_READ_PASS     10
#define BANK_RW_PASS       11
#define BANK_ENABLE_PASS   12
#define BANK_DISABLE_PASS  13
// Defines for the switch statements for data entry
#define MODE_TEXT          0
#define MODE_HEX           1
// Number of devices to list
#define MAXDEVICES         10
// Max number for data entry
#define MAX_LEN            256


int getNumber (int min, int max);

int menuSelect(int menu, uchar *SNum);

SMALLINT selectDevice(int numDevices, uchar AllDevices[][8]);

void printDeviceInfo(int portnum, uchar SNum[8]);

SMALLINT selectBank(SMALLINT bank, uchar *SNum);

void displayBankInformation (SMALLINT bank, int portnum, uchar SNum[8]);

SMALLINT dumpBankBlock (SMALLINT bank, int portnum, uchar SNum[8], int addr, int len);
SMALLINT dumpBankPage (SMALLINT bank, int portnum, uchar SNum[8], int pg);
SMALLINT dumpBankPagePacket(SMALLINT bank, int portnum, uchar SNum[8], int pg);

SMALLINT bankWriteBlock (SMALLINT bank, int portnum, uchar SNum[8], int addr, uchar *data, int length);
SMALLINT bankWritePacket(SMALLINT bank, int portnum, uchar SNum[8], int pg, uchar *data, int length);

SMALLINT optionSHAEE(SMALLINT bank, int portnum, uchar *SNum);

#endif // MEMUTIL_H_INCLUDED
