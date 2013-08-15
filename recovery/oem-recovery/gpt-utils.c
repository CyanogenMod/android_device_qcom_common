/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _LARGEFILE64_SOURCE /* enable lseek64() */

/******************************************************************************
 * INCLUDE SECTION
 ******************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "gpt-utils.h"
#include "sparse_crc32.h"



/******************************************************************************
 * DEFINE SECTION
 ******************************************************************************/
#define BLK_DEV_FILE    "/dev/block/mmcblk0"

/* list the names of the backed-up partitions to be swapped */
#define PTN_SWAP_LIST       "sbl1", "rpm", "tz", "aboot"
/* extension used for the backup partitions - tzbak, abootbak, etc. */
#define BAK_PTN_NAME_EXT    "bak"

#define LBA_SIZE    512

/* GPT defines */
#define GPT_SIGNATURE               "EFI PART"
#define HEADER_SIZE_OFFSET          12
#define HEADER_CRC_OFFSET           16
#define PRIMARY_HEADER_OFFSET       24
#define BACKUP_HEADER_OFFSET        32
#define FIRST_USABLE_LBA_OFFSET     40
#define LAST_USABLE_LBA_OFFSET      48
#define PENTRIES_OFFSET             72
#define PARTITION_COUNT_OFFSET      80
#define PENTRY_SIZE_OFFSET          84
#define PARTITION_CRC_OFFSET        88

#define TYPE_GUID_OFFSET            0
#define TYPE_GUID_SIZE              16
#define UNIQUE_GUID_OFFSET          16
#define FIRST_LBA_OFFSET            32
#define LAST_LBA_OFFSET             40
#define ATTRIBUTE_FLAG_OFFSET       48
#define PARTITION_NAME_OFFSET       56

#define MAX_GPT_NAME_SIZE           72



/******************************************************************************
 * MACROS
 ******************************************************************************/
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define GET_4_BYTES(ptr)    ((uint32_t) *((uint8_t *)(ptr)) | \
        ((uint32_t) *((uint8_t *)(ptr) + 1) << 8) | \
        ((uint32_t) *((uint8_t *)(ptr) + 2) << 16) | \
        ((uint32_t) *((uint8_t *)(ptr) + 3) << 24))

#define GET_8_BYTES(ptr)    ((uint64_t) *((uint8_t *)(ptr)) | \
        ((uint64_t) *((uint8_t *)(ptr) + 1) << 8) | \
        ((uint64_t) *((uint8_t *)(ptr) + 2) << 16) | \
        ((uint64_t) *((uint8_t *)(ptr) + 3) << 24) | \
        ((uint64_t) *((uint8_t *)(ptr) + 4) << 32) | \
        ((uint64_t) *((uint8_t *)(ptr) + 5) << 40) | \
        ((uint64_t) *((uint8_t *)(ptr) + 6) << 48) | \
        ((uint64_t) *((uint8_t *)(ptr) + 7) << 56))

#define PUT_4_BYTES(ptr, y)   *((uint8_t *)(ptr)) = (y) & 0xff; \
        *((uint8_t *)(ptr) + 1) = ((y) >> 8) & 0xff; \
        *((uint8_t *)(ptr) + 2) = ((y) >> 16) & 0xff; \
        *((uint8_t *)(ptr) + 3) = ((y) >> 24) & 0xff;



/******************************************************************************
 * TYPES
 ******************************************************************************/
enum gpt_instance {
    PRIMARY_GPT = 0,
    SECONDARY_GPT
};

enum boot_chain {
    NORMAL_BOOT = 0,
    BACKUP_BOOT
};

enum gpt_state {
    GPT_OK = 0,
    GPT_BAD_SIGNATURE,
    GPT_BAD_CRC
};



/******************************************************************************
 * FUNCTIONS
 ******************************************************************************/
/**
 *  ==========================================================================
 *
 *  \brief  Read/Write len bytes from/to block dev
 *
 *  \param [in] fd      block dev file descriptor (returned from open)
 *  \param [in] rw      RW flag: 0 - read, != 0 - write
 *  \param [in] offset  block dev offset [bytes] - RW start position
 *  \param [in] buf     Pointer to the buffer containing the data
 *  \param [in] len     RW size in bytes. Buf must be at least that big
 *
 *  \return  0 on success
 *
 *  ==========================================================================
 */
static int blk_rw(int fd, int rw, int64_t offset, uint8_t *buf, unsigned len)
{
    int r;

    if (lseek64(fd, offset, SEEK_SET) < 0) {
        fprintf(stderr, "block dev lseek64 %lld failed: %s\n", offset,
                strerror(errno));
        return -1;
    }

    if (rw)
        r = write(fd, buf, len);
    else
        r = read(fd, buf, len);

    if (r < 0)
        fprintf(stderr, "block dev %s failed: %s\n", rw ? "write" : "read",
                strerror(errno));
    else
        r = 0;

    return r;
}



/**
 *  ==========================================================================
 *
 *  \brief  Search within GPT for partition entry with the given name
 *  or it's backup twin (name-bak).
 *
 *  \param [in] ptn_name        Partition name to seek
 *  \param [in] pentries_start  Partition entries array start pointer
 *  \param [in] pentries_end    Partition entries array end pointer
 *  \param [in] pentry_size     Single partition entry size [bytes]
 *
 *  \return  First partition entry pointer that matches the name or NULL
 *
 *  ==========================================================================
 */
static uint8_t *gpt_pentry_seek(const char *ptn_name,
                                const uint8_t *pentries_start,
                                const uint8_t *pentries_end,
                                uint32_t pentry_size)
{
    char *pentry_name;
    unsigned len = strlen(ptn_name);

    for (pentry_name = (char *) (pentries_start + PARTITION_NAME_OFFSET);
         pentry_name < (char *) pentries_end; pentry_name += pentry_size) {
        char name8[MAX_GPT_NAME_SIZE / 2];
        unsigned i;

        /* Partition names in GPT are UTF-16 - ignoring UTF-16 2nd byte */
        for (i = 0; i < sizeof(name8); i++)
            name8[i] = pentry_name[i * 2];
        if (!strncmp(ptn_name, name8, len))
            if (name8[len] == 0 || !strcmp(&name8[len], BAK_PTN_NAME_EXT))
                return (uint8_t *) (pentry_name - PARTITION_NAME_OFFSET);
    }

    return NULL;
}



/**
 *  ==========================================================================
 *
 *  \brief  Swaps boot chain in GPT partition entries array
 *
 *  \param [in] pentries_start  Partition entries array start
 *  \param [in] pentries_end    Partition entries array end
 *  \param [in] pentry_size     Single partition entry size
 *
 *  \return  0 on success, 1 if no backup partitions found
 *
 *  ==========================================================================
 */
static int gpt_boot_chain_swap(const uint8_t *pentries_start,
                                const uint8_t *pentries_end,
                                uint32_t pentry_size)
{
    const char ptn_swap_list[][MAX_GPT_NAME_SIZE] = { PTN_SWAP_LIST };

    int backup_not_found = 1;
    unsigned i;

    for (i = 0; i < ARRAY_SIZE(ptn_swap_list); i++) {
        uint8_t *ptn_entry;
        uint8_t *ptn_bak_entry;
        uint8_t ptn_swap[TYPE_GUID_SIZE];

        ptn_entry = gpt_pentry_seek(ptn_swap_list[i], pentries_start,
                        pentries_end, pentry_size);
        if (ptn_entry == NULL)
            continue;

        ptn_bak_entry = gpt_pentry_seek(ptn_swap_list[i],
                        ptn_entry + pentry_size, pentries_end, pentry_size);
        if (ptn_bak_entry == NULL) {
            fprintf(stderr, "'%s' partition not backup - skip safe update\n",
                    ptn_swap_list[i]);
            continue;
        }

        /* swap primary <-> backup partition entries */
        memcpy(ptn_swap, ptn_entry + TYPE_GUID_OFFSET, TYPE_GUID_SIZE);
        memcpy(ptn_entry + TYPE_GUID_OFFSET,
                ptn_bak_entry + TYPE_GUID_OFFSET, TYPE_GUID_SIZE);
        memcpy(ptn_bak_entry + TYPE_GUID_OFFSET, ptn_swap, TYPE_GUID_SIZE);
        backup_not_found = 0;
    }

    return backup_not_found;
}



/**
 *  ==========================================================================
 *
 *  \brief  Sets secondary GPT boot chain
 *
 *  \param [in] fd    block dev file descriptor
 *  \param [in] boot  Boot chain to switch to
 *
 *  \return  0 on success
 *
 *  ==========================================================================
 */
static int gpt2_set_boot_chain(int fd, enum boot_chain boot)
{
    int64_t  gpt2_header_offset;
    uint64_t pentries_start_offset;
    uint32_t gpt_header_size;
    uint32_t pentry_size;
    uint32_t pentries_array_size;

    uint8_t  gpt_header[LBA_SIZE];
    uint8_t  *pentries;
    uint32_t crc;

    int r;

    gpt2_header_offset = lseek64(fd, 0, SEEK_END) - LBA_SIZE;
    if (gpt2_header_offset < 0) {
        fprintf(stderr, "Getting secondary GPT header offset failed: %s\n",
                strerror(errno));
        return -1;
    }

    /* Read primary GPT header from block dev */
    r = blk_rw(fd, 0, LBA_SIZE, gpt_header, LBA_SIZE);
    if (r)
        return r;

    pentries_start_offset =
        GET_8_BYTES(gpt_header + PENTRIES_OFFSET) * LBA_SIZE;
    pentry_size = GET_4_BYTES(gpt_header + PENTRY_SIZE_OFFSET);
    pentries_array_size =
        GET_4_BYTES(gpt_header + PARTITION_COUNT_OFFSET) * pentry_size;

    pentries = (uint8_t *) calloc(1, pentries_array_size);
    if (pentries == NULL) {
        fprintf(stderr,
                "Failed to allocate memory for GPT partition entries array\n");
        return -1;
    }

    /* Read primary GPT partititon entries array from block dev */
    r = blk_rw(fd, 0, pentries_start_offset, pentries, pentries_array_size);
    if (r)
        goto EXIT;

    crc = sparse_crc32(0, pentries, pentries_array_size);
    if (GET_4_BYTES(gpt_header + PARTITION_CRC_OFFSET) != crc) {
        fprintf(stderr, "Primary GPT partition entries array CRC invalid\n");
        r = -1;
        goto EXIT;
    }

    /* Read secondary GPT header from block dev */
    r = blk_rw(fd, 0, gpt2_header_offset, gpt_header, LBA_SIZE);
    if (r)
        goto EXIT;

    gpt_header_size = GET_4_BYTES(gpt_header + HEADER_SIZE_OFFSET);
    pentries_start_offset =
        GET_8_BYTES(gpt_header + PENTRIES_OFFSET) * LBA_SIZE;

    if (boot == BACKUP_BOOT) {
        r = gpt_boot_chain_swap(pentries, pentries + pentries_array_size,
                                pentry_size);
        if (r)
            goto EXIT;
    }

    crc = sparse_crc32(0, pentries, pentries_array_size);
    PUT_4_BYTES(gpt_header + PARTITION_CRC_OFFSET, crc);

    /* header CRC is calculated with this field cleared */
    PUT_4_BYTES(gpt_header + HEADER_CRC_OFFSET, 0);
    crc = sparse_crc32(0, gpt_header, gpt_header_size);
    PUT_4_BYTES(gpt_header + HEADER_CRC_OFFSET, crc);

    /* Write the modified GPT header back to block dev */
    r = blk_rw(fd, 1, gpt2_header_offset, gpt_header, LBA_SIZE);
    if (!r)
        /* Write the modified GPT partititon entries array back to block dev */
        r = blk_rw(fd, 1, pentries_start_offset, pentries,
                    pentries_array_size);

EXIT:
    free(pentries);
    return r;
}



/**
 *  ==========================================================================
 *
 *  \brief  Checks GPT state (header signature and CRC)
 *
 *  \param [in] fd      block dev file descriptor
 *  \param [in] gpt     GPT header to be checked
 *  \param [out] state  GPT header state
 *
 *  \return  0 on success
 *
 *  ==========================================================================
 */
static int gpt_get_state(int fd, enum gpt_instance gpt, enum gpt_state *state)
{
    int64_t gpt_header_offset;
    uint32_t gpt_header_size;
    uint8_t  gpt_header[LBA_SIZE];
    uint32_t crc;

    *state = GPT_OK;

    if (gpt == PRIMARY_GPT)
        gpt_header_offset = LBA_SIZE;
    else {
        gpt_header_offset = lseek64(fd, 0, SEEK_END) - LBA_SIZE;
        if (gpt_header_offset < 0)
            return -1;
    }

    if (blk_rw(fd, 0, gpt_header_offset, gpt_header, LBA_SIZE))
        return -1;

    if (memcmp(gpt_header, GPT_SIGNATURE, sizeof(GPT_SIGNATURE)))
        *state = GPT_BAD_SIGNATURE;

    gpt_header_size = GET_4_BYTES(gpt_header + HEADER_SIZE_OFFSET);

    crc = GET_4_BYTES(gpt_header + HEADER_CRC_OFFSET);
    /* header CRC is calculated with this field cleared */
    PUT_4_BYTES(gpt_header + HEADER_CRC_OFFSET, 0);
    if (sparse_crc32(0, gpt_header, gpt_header_size) != crc)
        *state = GPT_BAD_CRC;

    return 0;
}



/**
 *  ==========================================================================
 *
 *  \brief  Sets GPT header state (used to corrupt and fix GPT signature)
 *
 *  \param [in] fd     block dev file descriptor
 *  \param [in] gpt    GPT header to be checked
 *  \param [in] state  GPT header state to set (GPT_OK or GPT_BAD_SIGNATURE)
 *
 *  \return  0 on success
 *
 *  ==========================================================================
 */
static int gpt_set_state(int fd, enum gpt_instance gpt, enum gpt_state state)
{
    int64_t gpt_header_offset;
    uint32_t gpt_header_size;
    uint8_t  gpt_header[LBA_SIZE];
    uint32_t crc;

    if (gpt == PRIMARY_GPT)
        gpt_header_offset = LBA_SIZE;
    else {
        gpt_header_offset = lseek64(fd, 0, SEEK_END) - LBA_SIZE;
        if (gpt_header_offset < 0)
            return -1;
    }

    if (blk_rw(fd, 0, gpt_header_offset, gpt_header, LBA_SIZE))
        return -1;

    if (state == GPT_OK)
        memcpy(gpt_header, GPT_SIGNATURE, sizeof(GPT_SIGNATURE));
    else if (state == GPT_BAD_SIGNATURE)
        *gpt_header = 0;
    else
        return -1;

    gpt_header_size = GET_4_BYTES(gpt_header + HEADER_SIZE_OFFSET);

    /* header CRC is calculated with this field cleared */
    PUT_4_BYTES(gpt_header + HEADER_CRC_OFFSET, 0);
    crc = sparse_crc32(0, gpt_header, gpt_header_size);
    PUT_4_BYTES(gpt_header + HEADER_CRC_OFFSET, crc);

    if (blk_rw(fd, 1, gpt_header_offset, gpt_header, LBA_SIZE))
        return -1;

    return 0;
}



/**
 *  ==========================================================================
 *
 *  \brief  Prepare for certain boot partitions update stage
 *
 *  \param [in] stage  Update stage reached
 *
 *  \return  0 on success, < 0 on error
 *
 *  ==========================================================================
 */
int prepare_boot_update(enum boot_update_stage stage)
{
    int r, fd;
    enum gpt_state gpt_prim, gpt_second;
    enum boot_update_stage internal_stage;

    fd = open(BLK_DEV_FILE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Opening '%s' failed: %s\n", BLK_DEV_FILE,
                strerror(errno));
        return -1;
    }

    r = gpt_get_state(fd, PRIMARY_GPT, &gpt_prim) ||
        gpt_get_state(fd, SECONDARY_GPT, &gpt_second);
    if (r) {
        fprintf(stderr, "Getting GPT headers state failed\n");
        goto EXIT;
    }

    /* These 2 combinations are unexpected and unacceptable */
    if (gpt_prim == GPT_BAD_CRC || gpt_second == GPT_BAD_CRC) {
        fprintf(stderr, "GPT headers CRC corruption detected, aborting\n");
        r = -1;
        goto EXIT;
    }
    if (gpt_prim == GPT_BAD_SIGNATURE && gpt_second == GPT_BAD_SIGNATURE) {
        fprintf(stderr, "Both GPT headers corrupted, aborting\n");
        r = -1;
        goto EXIT;
    }

    /* Check internal update stage according GPT headers' state */
    if (gpt_prim == GPT_OK && gpt_second == GPT_OK)
        internal_stage = UPDATE_MAIN;
    else if (gpt_prim == GPT_BAD_SIGNATURE)
        internal_stage = UPDATE_BACKUP;
    else if (gpt_second == GPT_BAD_SIGNATURE)
        internal_stage = UPDATE_FINALIZE;
    else {
        fprintf(stderr, "Abnormal GPTs state: primary (%d), secondary (%d), "
                "aborting\n", gpt_prim, gpt_second);
        r = -1;
        goto EXIT;
    }

    /* Stage already set - ready for update, exitting */
    if ((int) stage == (int) internal_stage - 1)
        goto EXIT;
    /* Unexpected stage given */
    if (stage != internal_stage) {
        r = -1;
        goto EXIT;
    }

    switch (stage) {
    case UPDATE_MAIN:
        r = gpt2_set_boot_chain(fd, BACKUP_BOOT);
        if (r) {
            if (r < 0)
                fprintf(stderr,
                        "Setting secondary GPT to backup boot failed\n");
            /* No backup partitions - do not corrupt GPT, do not flag error */
            else
                r = 0;
            goto EXIT;
        }

        r = gpt_set_state(fd, PRIMARY_GPT, GPT_BAD_SIGNATURE);
        if (r) {
            fprintf(stderr, "Corrupting primary GPT header failed\n");
            goto EXIT;
        }

        break;
    case UPDATE_BACKUP:
        r = gpt_set_state(fd, PRIMARY_GPT, GPT_OK);
        if (r) {
            fprintf(stderr, "Fixing primary GPT header failed\n");
            goto EXIT;
        }

        r = gpt_set_state(fd, SECONDARY_GPT, GPT_BAD_SIGNATURE);
        if (r) {
            fprintf(stderr, "Corrupting secondary GPT header failed\n");
            goto EXIT;
        }

        break;
    case UPDATE_FINALIZE:
        r = gpt2_set_boot_chain(fd, NORMAL_BOOT);
        if (r < 0) {
            fprintf(stderr, "Setting secondary GPT to normal boot failed\n");
            goto EXIT;
        }

        r = gpt_set_state(fd, SECONDARY_GPT, GPT_OK);
        if (r) {
            fprintf(stderr, "Fixing secondary GPT header failed\n");
            goto EXIT;
        }

        break;
    default:;
    }

EXIT:
    fsync(fd);
    close(fd);

    return r;
}
