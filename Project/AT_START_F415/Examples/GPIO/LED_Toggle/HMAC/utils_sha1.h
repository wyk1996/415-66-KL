/*******************************************************************************
 *          Copyright (c) 2020-2050, 66ifuel Co., Ltd.
 *                              All Right Reserved.
 * @file utils_sha1.h
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

#ifndef UTILS_SHA1_H_
#define UTILS_SHA1_H_

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"

/**
 * \brief          SHA-1 context structure
 */
typedef struct {
    uint32_t total[2];          /*!< number of bytes processed  */
    uint32_t state[5];          /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */
} iot_sha1_context;

/**
 * \brief          Initialize SHA-1 context
 *
 * \param ctx      SHA-1 context to be initialized
 */
void utils_sha1_init(iot_sha1_context *ctx);

/**
 * \brief          Clear SHA-1 context
 *
 * \param ctx      SHA-1 context to be cleared
 */
void utils_sha1_free(iot_sha1_context *ctx);

/**
 * \brief          Clone (the state of) a SHA-1 context
 *
 * \param dst      The destination context
 * \param src      The context to be cloned
 */
void utils_sha1_clone(iot_sha1_context *dst,
                      const iot_sha1_context *src);

/**
 * \brief          SHA-1 context setup
 *
 * \param ctx      context to be initialized
 */
void utils_sha1_starts(iot_sha1_context *ctx);

/**
 * \brief          SHA-1 process buffer
 *
 * \param ctx      SHA-1 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void utils_sha1_update(iot_sha1_context *ctx, const unsigned char *input, size_t ilen);

/**
 * \brief          SHA-1 final digest
 *
 * \param ctx      SHA-1 context
 * \param output   SHA-1 checksum result
 */
void utils_sha1_finish(iot_sha1_context *ctx, unsigned char output[20]);

/* Internal use */
void utils_sha1_process(iot_sha1_context *ctx, const unsigned char data[64]);

/**
 * \brief          Output = SHA-1( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SHA-1 checksum result
 */
void utils_sha1(const unsigned char *input, size_t ilen, unsigned char output[20]);

#endif
