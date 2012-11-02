
#ifndef __W1_USERSPACE_H
#define __W1_USERSPACE_H


#include <asm/byteorder.h>

/*
 * It's originally inside w1.h ----------------------------------------------
 */
typedef struct w1_reg_num
{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u64	family:8,
		id:48,
		crc:8;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u64	crc:8,
		id:48,
		family:8;
#else
#error "Please fix <asm/byteorder.h>"
#endif
} w1_slave_rn;


#define W1_EMPTY_REG_NUM     {.family = 0, .id = 0, .crc = 0}



typedef __u32 w1_master_id;


/*
* Below are utility functions ----------------------------------------------
*/


#ifdef __cplusplus
extern "C" {
#endif


/**
* OK return TRUE, Error return FALSE.
* The input-output parameter "outputStr" must be at least 20.
* It's format: %02x.%012llx.%02x
*/
BOOL w1_reg_num__to_string(struct w1_reg_num * w1RegNum, char * outputStr);

/**
* OK return TRUE, Error return FALSE.
* It's format: %02x.%012llx.%02x
*/
BOOL w1_reg_num__from_string(char * inputStr, struct w1_reg_num * w1RegNum);


BOOL w1_slave_rn__is_empty(w1_slave_rn rn);


BOOL w1_slave_rn__are_equal(w1_slave_rn rn1, w1_slave_rn rn2);


#ifdef __cplusplus
}
#endif



#endif /* __W1_USERSPACE_H */
