/*
 *
 * Copyright (c) 2011 Deven Fan <deven.fan@gmail.com>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#include "sh_util.h"

static BOOL get_hexchar(BYTE fourBits, char * hexChar)
{
    if(fourBits >= 0 && fourBits <= 9)
        *hexChar = '0' + fourBits;
    else if(fourBits >= 10 && fourBits <= 15)
        *hexChar = 'A' + fourBits - 10;
    else
        return FALSE;
    return TRUE;
}

static BOOL get_fourBits(char hexChar, BYTE * fourBits)
{
    if(hexChar >= '0' && hexChar <= '9')
        *fourBits = hexChar - '0';
    else if(hexChar >= 'a' && hexChar <= 'f')
        *fourBits = hexChar - 'a';
    else if(hexChar >= 'A' && hexChar <= 'F')
        *fourBits = hexChar - 'A';
    else
        return FALSE;
    return TRUE;
}

BOOL is_hexstr(char * str, int length)
{
    int i = 0;
    for(i = 0; i < length; i++)
    {
        if(str[i] >= '0' && str[i] <= '9')
            continue;
        else if(str[i] >= 'a' && str[i] <= 'f')
            continue;
        else if(str[i] >= 'A' && str[i] <= 'F')
            continue;
        else
            break;
    }
    return (i == length) ? TRUE : FALSE;
}


BOOL convert_bytes_to_hexstr(BYTE * bytesIn, int bytesOffset, int bytesLen, char * hexstrOut, int * hexstrLen)
{
    int i, j;
    BYTE b, h, l;
    for(i = bytesOffset, j = 0; i < (bytesOffset + bytesLen);)
    {
        b = bytesIn[i++];
        h = (b & 0xF0) >> 4;
        l = b & 0x0F;
        get_hexchar(h, hexstrOut + j++);
        get_hexchar(l, hexstrOut + j++);
    }
    *hexstrLen = j;
    return TRUE; //TODO
}

BOOL convert_hexstr_to_bytes(char * hexstrIn, int hexstrLen, BYTE * bytesOut, int * bytesLen)
{
    BYTE h, l;
    int i, j;
    for(i = 0, j = 0; i < hexstrLen;)
    {
        get_fourBits(hexstrIn[i++], &h);
        get_fourBits(hexstrIn[i++], &l);
        bytesOut[j++] = (h << 4) | l;
    }
    *bytesLen = j;
    return TRUE; //TODO
}
