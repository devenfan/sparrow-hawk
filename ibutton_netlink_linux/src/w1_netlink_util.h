#ifndef W1_NETLINK_UTIL_H_INCLUDED
#define W1_NETLINK_UTIL_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif




/**
* OK return TRUE, Error return FALSE.
* The input-output parameter "outputStr" must be at least 20.
*/
BOOL describe_w1_msg_type(int msgType, char * outputStr);


/**
* OK return TRUE, Error return FALSE.
* The input-output parameter "outputStr" must be at least 20.
*/
BOOL describe_w1_cmd_type(int cmdType, char * outputStr);


/**
* OK return TRUE, Error return FALSE.
* The input-output parameter "outputStr" must be at least 20.
* It's formart Like: %02x.%012llx.%02x
*/
BOOL describe_w1_reg_num(struct w1_reg_num * w1RegNum, char * outputStr);


void print_cnmsg(const struct cn_msg * cnmsg);


void print_w1msg(const struct w1_netlink_msg * w1msg);


void print_w1cmd(const struct w1_netlink_cmd * w1cmd);



#ifdef __cplusplus
}
#endif



#endif // W1_NETLINK_UTIL_H_INCLUDED
