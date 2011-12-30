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

#ifndef __SH_UTIL_H
#define __SH_UTIL_H


//#include "sh_types.h"

BOOL is_hexstr(char * str, int length);

BOOL convert_bytes_to_hexstr(BYTE * bytesIn, int bytesOffset, int bytesLen, char * hexstrOut, int * hexstrLen);

BOOL convert_hexstr_to_bytes(char * hexstrIn, int hexstrLen, BYTE * bytesOut, int * bytesLen);


#endif // __SH_UTIL_H
