
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sh_types.h"
#include "sh_util.h"
#include "w1_userspace.h"



BOOL w1_reg_num__to_string(struct w1_reg_num * w1RegNum, char * outputStr)
{
    if(w1RegNum == NULL || outputStr == NULL) return FALSE;

    //unsigned long long -> %llx
    sprintf(outputStr, "%02X.%012llX.%02X", w1RegNum->family,
            (unsigned long long)w1RegNum->id, w1RegNum->crc);

    return TRUE;
}

BOOL w1_reg_num__from_string(char * inputStr, struct w1_reg_num * w1RegNum)
{
    if(inputStr == NULL || w1RegNum == NULL) return FALSE;

    int dataOutLen = 0;
    BYTE dataOut[8];

    memset(dataOut, 0, 8);

    //Format: %02X.%012llX.%02X; Length: 2+1+12+1+2
    convert_hexstr_to_bytes(inputStr, 2, dataOut, &dataOutLen);             //1 byte
    convert_hexstr_to_bytes(inputStr +2+1, 12, dataOut + 1, &dataOutLen);    //6 bytes
    convert_hexstr_to_bytes(inputStr +2+1 +12+1, 2, dataOut + 7, &dataOutLen);    //1 byte

    memcpy(w1RegNum, dataOut, 8);

    return TRUE;
}


BOOL w1_slave_rn__is_empty(w1_slave_rn rn)
{
    w1_slave_rn empty_rn = W1_EMPTY_REG_NUM;

    return (memcmp(&rn, &empty_rn, sizeof(w1_slave_rn)) == 0) ? TRUE : FALSE;
}


BOOL w1_slave_rn__are_equal(w1_slave_rn rn1, w1_slave_rn rn2)
{
    /*
    char salveIDStr[20];

    describe_w1_reg_num(&rn1, salveIDStr);
    Debug(salveIDStr);

    describe_w1_reg_num(&rn2, salveIDStr);
    Debug(salveIDStr);
    */

    return (memcmp(&rn1, &rn2, sizeof(w1_slave_rn)) == 0) ? TRUE : FALSE;
}
