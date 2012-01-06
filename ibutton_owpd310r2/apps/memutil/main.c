
#include <stdio.h>
#include <ctype.h>

#include "ownet.h"
#include "rawmem.h"
#include "mbshaee.h"
#include "mbeprom.h"
#include "mbee77.h"
#include "pw77.h"

#include "memutil.h"

// Global necessary for screenio
//int VERBOSE;
//uchar AllSN[MAXDEVICES][8];


int main(int argc, char **argv)
{
   int len, addr, page, answer, i;
   int done      = FALSE;
   SMALLINT  bank = 1;
   uchar     data[552];
   int portnum = 0;
   uchar AllSN[MAXDEVICES][8];
   int NumDevices;
   int owd;
   char msg[132];


      // check for required port name
   if (argc != 2)
   {
      sprintf(msg,"1-Wire Net name required on command line!\n"
                  " (example: \"COM1\" (Win32 DS2480),\"/dev/cua0\" "
                  "(Linux DS2480),\"{1,5}\" (Win32 TMEX)\n");
      printf("%s\n",msg);
      return 0;
   }

   printf("\n1-Wire Memory utility console application Version 0.01\n");

   if((portnum = owAcquireEx(argv[1])) < 0)
   {
      OWERROR_DUMP(stdout);
      return 0;
   }

   {
      // loop to do menu
      do
      {

         // Main menu
         switch (menuSelect(MAIN_MENU,&AllSN[0][0]))
         {

            case MAIN_SELECT_DEVICE :

               // find all parts
               // loop to find all of the devices up to MAXDEVICES
               NumDevices = 0;
               do
               {
                  // perform the search
                  if (!owNext(portnum,TRUE, FALSE))
                     break;

                  owSerialNum(portnum,AllSN[NumDevices], TRUE);
                  NumDevices++;
               }
               while (NumDevices < (MAXDEVICES - 1));

               /* for test devices without a serial number
               if(NumDevices == 0)
               {
                  for(i=0;i<8;i++)
                     AllSN[0][i] = 0x00;
                  NumDevices++;
               }*/

               // select a device
               owd = selectDevice(NumDevices,&AllSN[0]);

               // display device info
               printDeviceInfo(portnum,&AllSN[owd][0]);

               // select a bank
               bank = selectBank(bank, &AllSN[owd][0]);

               if((AllSN[owd][0] == 0x33) || (AllSN[owd][0] == 0xB3))
                  bank = optionSHAEE(bank,portnum,&AllSN[owd][0]);

               // display bank information
               displayBankInformation(bank,portnum,&AllSN[owd][0]);

               // loop on bank menu
               do
               {
                  switch (menuSelect(BANK_MENU,&AllSN[owd][0]))
                  {

                     case BANK_INFO :
                        // display bank information
                        displayBankInformation(bank,portnum,&AllSN[owd][0]);
                        break;

                     case BANK_READ_BLOCK :
                        // read a block
                        printf("Enter the address to start reading: ");
                        addr = getNumber(0, (owGetSize(bank,&AllSN[owd][0])-1));
                        printf("\n");

                        printf("Enter the length of data to read: ");
                        len = getNumber(0, owGetSize(bank, &AllSN[owd][0]));
                        printf("\n");

                        if(!dumpBankBlock(bank,portnum,&AllSN[owd][0],addr,len))
                           OWERROR_DUMP(stderr);
                        break;

                     case BANK_READ_PAGE :
                        printf("Enter the page number to read:  ");

                        page = getNumber(0, (owGetNumberPages(bank,&AllSN[owd][0])-1));

                        printf("\n");

                        if(!dumpBankPage(bank,portnum,&AllSN[owd][0],page))
                           OWERROR_DUMP(stderr);

                        break;

                     case BANK_READ_UDP :
                        printf("Enter the page number to read: ");

                        page = getNumber(0, (owGetNumberPages(bank,&AllSN[owd][0])-1));

                        printf("\n");

                        if(!dumpBankPagePacket(bank,portnum,&AllSN[owd][0],page))
                           OWERROR_DUMP(stderr);
                        break;

                     case BANK_WRITE_BLOCK :
                        // write a block
                        printf("Enter the address to start writing: ");

                        addr = getNumber(0, (owGetSize(bank,&AllSN[owd][0])-1));

            				if(menuSelect(ENTRY_MENU,&AllSN[owd][0]) == MODE_TEXT)
            					len = getData(data,MAX_LEN,MODE_TEXT);
            				else
            					len = getData(data,MAX_LEN,MODE_HEX);

                        if(!bankWriteBlock(bank,portnum,&AllSN[owd][0],addr,data,len))
                        {
                           OWERROR_DUMP(stderr);
                           break;
                        }

                        if(owCanRedirectPage(bank,&AllSN[owd][0]))
                        {
                           printf("Enter if you want to redirect page (0 no, 1 yes): ");

                           answer = getNumber(0,1);

                           if(answer)
                           {
                              printf("What page would you like to redirect:");

                              page = getNumber(0,255);

                              printf("Where would you like to redirect:");

                              addr = getNumber(0,255);

                              if(!redirectPage(bank,portnum,&AllSN[owd][0],page,addr))
                              {
                                 OWERROR_DUMP(stderr);
                                 break;
                              }
                           }
                        }

                        if(owCanLockPage(bank,&AllSN[owd][0]))
                        {
                           printf("Enter if you want to lock page (0 no, 1 yes):");

                           answer = getNumber(0,1);

                           if(answer)
                           {
                              printf("What page would you like to lock?");

                              page = getNumber(0,255);

                              if(!lockPage(bank,portnum,&AllSN[owd][0],page))
                              {
                                 OWERROR_DUMP(stderr);
                                 break;
                              }
                           }
                        }

                        if(owCanLockRedirectPage(bank,&AllSN[owd][0]))
                        {
                           printf("Enter if you want to lock redirected page (0 no, 1 yes):");

                           answer = getNumber(0,1);

                           if(answer)
                           {
                              printf("Which redirected page do you want to lock:");

                              page = getNumber(0,255);

                              if(!lockRedirectPage(bank,portnum,&AllSN[owd][0],page))
                              {
                                 OWERROR_DUMP(stderr);
                                 break;
                              }
                           }
                        }
                        break;

                     case BANK_WRITE_UDP :
                        printf("Enter the page number to write a UDP to: ");

                        page = getNumber(0, (owGetNumberPages(bank,&AllSN[owd][0])-1));

            				if(menuSelect(ENTRY_MENU,&AllSN[owd][0]) == MODE_TEXT)
            					len = getData(data,MAX_LEN,MODE_TEXT);
            				else
            					len = getData(data,MAX_LEN,MODE_HEX);

                        if(!bankWritePacket(bank,portnum,&AllSN[owd][0],page,data,len))
                           OWERROR_DUMP(stderr);
                        break;

                     case BANK_BM_READ_PASS:
                        printf("Enter the 8 byte read only password if less 0x00 will be filled in.");

            				if(menuSelect(ENTRY_MENU,&AllSN[owd][0]) == MODE_TEXT)
            					len = getData(data,8,MODE_TEXT);
            				else
            					len = getData(data,8,MODE_HEX);

                        if(len != 8)
                        {
                           for(i=len;i<8;i++)
                              data[i] = 0x00;
                        }

                        if(!owSetBMReadOnlyPassword(portnum,&AllSN[owd][0],data))
                           OWERROR_DUMP(stderr);

                        break;

                     case BANK_BM_RW_PASS:
                        printf("Enter the 8 byte read/write password if less 0x00 will be filled in.");

            				if(menuSelect(ENTRY_MENU,&AllSN[owd][0]) == MODE_TEXT)
            					len = getData(data,8,MODE_TEXT);
            				else
            					len = getData(data,8,MODE_HEX);

                        if(len != 8)
                        {
                           for(i=len;i<8;i++)
                              data[i] = 0x00;
                        }

                        if(!owSetBMReadWritePassword(portnum,&AllSN[owd][0],data))
                           OWERROR_DUMP(stderr);

                        break;

                     case BANK_READ_PASS:
                        printf("Enter the 8 byte read only password if less 0x00 will be filled in.");

            				if(menuSelect(ENTRY_MENU,&AllSN[owd][0]) == MODE_TEXT)
            					len = getData(data,8,MODE_TEXT);
            				else
            					len = getData(data,8,MODE_HEX);

                        if(len != 8)
                        {
                           for(i=len;i<8;i++)
                              data[i] = 0x00;
                        }

                        if(!owSetReadOnlyPassword(portnum,&AllSN[owd][0],data))
                           OWERROR_DUMP(stderr);

                        break;

                     case BANK_RW_PASS:
                        printf("Enter the 8 byte read/write password if less 0x00 will be filled in.");

            				if(menuSelect(ENTRY_MENU,&AllSN[owd][0]) == MODE_TEXT)
            					len = getData(data,8,MODE_TEXT);
            				else
            					len = getData(data,8,MODE_HEX);

                        if(len != 8)
                        {
                           for(i=len;i<8;i++)
                              data[i] = 0x00;
                        }

                        if(!owSetReadWritePassword(portnum,&AllSN[owd][0],data))
                           OWERROR_DUMP(stderr);

                        break;

                     case BANK_ENABLE_PASS:
                        if(!owSetPasswordMode(portnum,&AllSN[owd][0],ENABLE_PSW))
                           OWERROR_DUMP(stderr);
                        break;

                     case BANK_DISABLE_PASS:
                        if(!owSetPasswordMode(portnum,&AllSN[owd][0],DISABLE_PSW))
                           OWERROR_DUMP(stderr);
                        break;

                     case BANK_NEW_BANK :
                        // select a bank
                        bank = selectBank(bank,&AllSN[owd][0]);

                        if((AllSN[owd][0] == 0x33) || (AllSN[owd][0] == 0xB3))
                           bank = optionSHAEE(bank,portnum,&AllSN[owd][0]);

                        // display bank information
                        displayBankInformation(bank,portnum,&AllSN[owd][0]);
                        break;

                     case BANK_MAIN_MENU :
                        done = TRUE;
                        break;
                  }
               }
               while (!done);

               done = FALSE;
               break;

            case MAIN_QUIT :
               done = TRUE;
               break;

         }  // Main menu switch
      }
      while (!done);  // loop to do menu

      owRelease(portnum);
   }  // else for owAcquire

   return 1;
}

