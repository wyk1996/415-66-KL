/*******************************************************************************
 *          Copyright (c) 2020-2050, 66ifuel Co., Ltd.
 *                              All Right Reserved.
 * @file utils_hmac.h
 * @note 
 * @brief 
 * 
 * @author   
 * @date     2021-05-02
 * @version  V1.0.0
 * 
 * @Description 
 *  
 * @note History:        
 * @note     <author>   <time>    <version >   <desc>
 * @note  
 * @warning
 *******************************************************************************/
#ifndef UTILS_HMAC_H_
#define UTILS_HMAC_H_

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"

void utils_hmac_md5(const char *msg, int msg_len, char *digest, const char *key, int key_len);
void utils_hmac_sha1(const char *msg, int msg_len, char *digest, const char *key, int key_len);
int base64_decode( const char * base64, unsigned char * bindata );
char *base64_encode(char *bindata, char *base64, int binlength);

#endif

