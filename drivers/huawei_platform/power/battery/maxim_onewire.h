#ifndef MAXIM_ONEWIRE_H
#define MAXIM_ONEWIRE_H

#include "onewire_common.h"
#include "onewire_phy_common.h"

/* onewire common interface */
typedef struct {
    onewire_time_request ow_trq;
    unsigned int program_ms;
    unsigned int secret_program_ms;
    unsigned int compute_mac_ms;
}maxim_time_request;

typedef struct{
    unsigned char *rom_id;
    unsigned char *block_status;
    unsigned char *personality;
    unsigned char *sram;
    unsigned char *user_memory;
    unsigned char *mac;
    unsigned int  ic_type;
    unsigned int  validity;
    unsigned int  rom_id_length;
    unsigned int  block_number;
    unsigned int  block_size;
    unsigned int  personality_length;
    unsigned int  sram_length;
    unsigned int  page_number;
    unsigned int  page_size;
    unsigned int  mac_length;
    spinlock_t    lock;
}maxim_onewire_mem;

struct __maxim_onewire_ic_des;

struct __maxim_mem_ops{
    int (*get_rom_id)(struct __maxim_onewire_ic_des *);
    int (*get_personality)(struct __maxim_onewire_ic_des *);
    int (*get_user_memory)(struct __maxim_onewire_ic_des *, unsigned char);
    int (*set_user_memory)(struct __maxim_onewire_ic_des *, unsigned char *, unsigned char, unsigned char);
    int (*get_sram)(struct __maxim_onewire_ic_des *);
    int (*set_sram)(struct __maxim_onewire_ic_des *, unsigned char *);
    int (*get_mac)(struct __maxim_onewire_ic_des *, unsigned char, unsigned char);
    int (*valid_mem_ops)(struct __maxim_onewire_ic_des *, struct platform_device *);
};

/* maxim_onewire_ic_des--maxim onewire ic memory and commution operations */
struct __maxim_onewire_ic_des{
    maxim_time_request trq;
    maxim_onewire_mem memory;
    struct __maxim_mem_ops mem_ops;
    onewire_phy_ops phy_ops;
};

typedef struct __maxim_mem_ops maxim_mem_ops;
typedef struct __maxim_onewire_ic_des maxim_onewire_ic_des;

#define DOUBLE(x)                                           ((x)<<1)

/* Battery information validity macro */
#define ROM_ID_VALIDITY_BIT                                 0x01
#define PERSONALITY_VALIDITY_BIT                            0x02
#define SRAM_VALIDITY_BIT                                   0x04
#define STATUS_VALIDITY_BIT                                 0x08
#define USER_MEMORY_VALIDITY_BIT                            0x10
#define MAC_VALIDITY_BIT                                    0x20

/* Maxim ROM */
#define MAXIM_ROM_SIZE                                      8
#define MAXIM_SEGMENT_SIZE                                  4
#define MAXIM_SECRET_SIZE                                   32
#define MAXIM_MAC256_SIZE                                   32

/* Slave presence signal is low */
#define NO_SLAVE_RESPONSE(x)                                ((x)!=0)

/* MAXIM 1-wire memory and SHA function command */
#define WRITE_MEMORY                                        0x55
#define READ_MEMORY                                         0xF0
#define WRITE_BLOCK_PROTECTION                              0xC3
#define READ_STAUS                                          0xAA
#define READ_WRITE_SCRATCHPAD                               0x0F
#define LOAD_AND_LOCK_SECRET                                0x33
#define COMPUTE_AND_LOCK_SECRET                             0x3C
#define COMPUTE_AND_READ_PAGE_MAC                           0xA5
#define AUTHENTICATED_WRITE_MEMORY                          0x5A
#define AUTHENTICATED_WRITE_BLOCK_PROTECTION                0xCC

/* MAXIM 1-wire rom function command */
#define READ_ROM                                            0x33
#define MATCH_ROM                                           0x55
#define SEARCH_ROM                                          0xF0
#define SKIP_ROM                                            0xCC
#define RESUME_COMMAND                                      0xA5

/* Command Parameters */
#define SEGMENT_OFFSET                                      0x05
#define MAXIM_PAGE_MASK                                     0x01
#define SEGMENT_MASK                                        0x07
#define ANONYMOUS_MODE                                      0xE0
#define READ_MODE                                           0x0F
#define WRITE_MODE                                          0x00
#define PERSONALITY_CONFIG                                  0xE0
#define ONEWIRE_RELEASE                                     0xAA
#define BLOCK_MASK                                          0x03
#define PROTECTION_MASK                                     0xF0
#define READ_PROTECTION                                     0x80
#define WRITE_PROTECTION                                    0x40
#define EPROM_EMULATION_MODE                                0x20
#define AUTHENTICATED_PROTECTION                            0x10
#define MAXIM_PAGE0                                         0
#define MAXIM_PAGE1                                         1
#define MAXIM_SEGMENT0                                      0
#define MAXIM_SEGMENT1                                      1
#define MAXIM_SEGMENT2                                      2
#define MAXIM_SEGMENT3                                      3
#define MAXIM_SEGMENT4                                      4
#define MAXIM_SEGMENT5                                      5
#define MAXIM_SEGMENT6                                      6
#define MAXIM_SEGMENT7                                      7
#define MAXIM_MAN_ID_SIZE                                   2
#define MAXIM_MAN_ID_OFFSET                                 2

/* Command success response */
#define COMMAND_SUCCESS                                     0xAA

/* 1-wire function operation return signals */
#define ONEWIRE_NO_RESPONSE                                 0x11
#define ONEWIRE_CRC8_ERR                                    0x12
#define ONEWIRE_CRC16_ERR                                   0x13

/* SHA ERROR */
#define SHA_SUCCESS                                         0x00
#define SHA_PARA_ERR                                        0x01
#define SHA_MALLOC_ERR                                      0x02
#define SHA_LENGTH_ERR                                      0x03


/* MAC computation mode */
#define MAXIM_SEGMENT_4BYTES                                4
#define MAXIM_64BYTES                                       64
#define MAXIM_128BYTES                                      128

/* CRC result */
#define MAXIM_CRC16_RESULT                                  0xB001
#define MAXIM_CRC8_RESULT                                   0


/*
 *For SHA-1, SHA-224  and SHA-256, each message block has 512 bits, which are
 *represented as a sequence of sixteen 32-bit words.
 */
#define SHA256_BLOCKSIZE_512BITS                            512
#define SHA256_WORDSIZE_32BITS                              32
#define SHA256_WORDSIZE_4BYTES                              4

/* MAC input data structure */
#define AUTH_MAC_ROM_ID_OFFSET                              96
#define AUTH_MAC_PAGE_OFFSET                                0
#define AUTH_MAC_SRAM_OFFSET                                32
#define AUTH_MAC_KEY_OFFSET                                 64
#define AUTH_MAC_PAGE_NUM_OFFSET                            106
#define AUTH_MAC_MAN_ID_OFFSET                              104
#define MAX_MAC_SOURCE_SIZE                                 128

/* detect onewire slaves and notify all slaves */
#define ONEWIRE_NOTIFY_ALL_SLAVES(x)                                                            \
    if(NO_SLAVE_RESPONSE(x->reset())){                                                          \
        hwlog_err("1-wire maxim: no slave device response, found in %s", __func__);             \
        return ONEWIRE_NO_SLAVE;                                                                \
    }                                                                                           \
    x->write_byte(SKIP_ROM)

/* Maxim CRC16 check */
#define DO_CRC16(a,b)                                                                           \
    do{                                                                                         \
        if (check_crc16(a,b) != MAXIM_CRC16_RESULT){                                            \
            hwlog_err("maxim: CRC16 error, %s", __func__);                                      \
            return ONEWIRE_COM_FAIL;                                                            \
        }                                                                                       \
    }while(0)

/* Maxim CRC8 check */
#define CRC8_ERROR_PROCESS(x)                                                                   \
    do{                                                                                         \
        if(x != MAXIM_CRC8_RESULT){                                                             \
            hwlog_err("maxim: CRC8 error, %s", __func__);                                       \
            return ONEWIRE_COM_FAIL;                                                            \
        }                                                                                       \
    }while(0)

/* Command response process */
#define MAXIM_COMMAND_FAIL_PROCESS(x)                                                           \
    do{                                                                                         \
        if(x != COMMAND_SUCCESS){                                                               \
            hwlog_err("maxim: Error command indicator %x in %s", x, __func__);                  \
            return ONEWIRE_COM_FAIL;                                                            \
        }                                                                                       \
    }while(0)

/* dts read property error process*/
#define ONEWIRE_DTS_READ_ERROR(x)                                                               \
    do{                                                                                         \
        if(ret){                                                                                \
            hwlog_err("Battery-driver: DTS do not have "x", needed in %s.",__func__);          \
            return BATTERY_DRIVER_FAIL;                                                         \
        }                                                                                       \
    }while(0)

int maxim_onewire_register(maxim_onewire_ic_des *, struct platform_device *);

#endif