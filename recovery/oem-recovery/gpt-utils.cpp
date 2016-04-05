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
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/fs.h>
#include "gpt-utils.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "sparse_crc32.h"
#ifdef __cplusplus
}
#endif


/******************************************************************************
 * DEFINE SECTION
 ******************************************************************************/
#define BLK_DEV_FILE    "/dev/block/mmcblk0"
#define UFS_DEV_DIR     "/dev/block/sda"
#define BOOT_DEV_DIR    "/dev/block/bootdevice/by-name"
/* list the names of the backed-up partitions to be swapped */
#define PTN_SWAP_LIST       "sbl1", "rpm", "tz", "aboot", "hyp", "lksecapp", "keymaster", "cmnlib", "cmnlib64", "pmic"
/* extension used for the backup partitions - tzbak, abootbak, etc. */
#define BAK_PTN_NAME_EXT    "bak"

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
#define PTN_ENTRY_SIZE              128
#define UNIQUE_GUID_OFFSET          16
#define FIRST_LBA_OFFSET            32
#define LAST_LBA_OFFSET             40
#define ATTRIBUTE_FLAG_OFFSET       48
#define PARTITION_NAME_OFFSET       56

#define MAX_GPT_NAME_SIZE           72
#define MAX_PATH_LEN                255
#define MAX_LUNS                    26
//This will allow us to get the root lun path from the path to the partition.
//i.e: from /dev/block/sdaXXX get /dev/block/sda. The assumption here is that
//the boot critical luns lie between sda to sdz which is acceptable because
//only user added external disks,etc would lie beyond that limit which do not
//contain partitions that interest us here.
#define PATH_TRUNCATE_LOC 14

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
//List of LUN's containing boot critical images.
//Required in the case of UFS devices
struct update_data {
     char lun_list[MAX_LUNS][MAX_PATH_LEN];
     int num_valid_entries;
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
        uint8_t ptn_swap[PTN_ENTRY_SIZE];

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
        memcpy(ptn_swap, ptn_entry, PTN_ENTRY_SIZE);
        memcpy(ptn_entry, ptn_bak_entry, PTN_ENTRY_SIZE);
        memcpy(ptn_bak_entry, ptn_swap, PTN_ENTRY_SIZE);
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

    uint8_t *gpt_header = NULL;
    uint8_t  *pentries = NULL;
    uint32_t crc;
    uint32_t blk_size = 0;
    int r;

    if (ioctl(fd, BLKSSZGET, &blk_size) != 0) {
            fprintf(stderr, "Failed to get GPT device block size: %s\n",
                            strerror(errno));
            r = -1;
            goto EXIT;
    }
    gpt_header = (uint8_t*)malloc(blk_size);
    if (!gpt_header) {
            fprintf(stderr, "Failed to allocate memory to hold GPT block\n");
            r = -1;
            goto EXIT;
    }
    gpt2_header_offset = lseek64(fd, 0, SEEK_END) - blk_size;
    if (gpt2_header_offset < 0) {
        fprintf(stderr, "Getting secondary GPT header offset failed: %s\n",
                strerror(errno));
        r = -1;
        goto EXIT;
    }

    /* Read primary GPT header from block dev */
    r = blk_rw(fd, 0, blk_size, gpt_header, blk_size);

    if (r) {
            fprintf(stderr, "Failed to read primary GPT header from blk dev\n");
            goto EXIT;
    }
    pentries_start_offset =
        GET_8_BYTES(gpt_header + PENTRIES_OFFSET) * blk_size;
    pentry_size = GET_4_BYTES(gpt_header + PENTRY_SIZE_OFFSET);
    pentries_array_size =
        GET_4_BYTES(gpt_header + PARTITION_COUNT_OFFSET) * pentry_size;

    pentries = (uint8_t *) calloc(1, pentries_array_size);
    if (pentries == NULL) {
        fprintf(stderr,
                    "Failed to alloc memory for GPT partition entries array\n");
        r = -1;
        goto EXIT;
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
    r = blk_rw(fd, 0, gpt2_header_offset, gpt_header, blk_size);
    if (r)
        goto EXIT;

    gpt_header_size = GET_4_BYTES(gpt_header + HEADER_SIZE_OFFSET);
    pentries_start_offset =
        GET_8_BYTES(gpt_header + PENTRIES_OFFSET) * blk_size;

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
    r = blk_rw(fd, 1, gpt2_header_offset, gpt_header, blk_size);
    if (!r)
        /* Write the modified GPT partititon entries array back to block dev */
        r = blk_rw(fd, 1, pentries_start_offset, pentries,
                    pentries_array_size);

EXIT:
    if(gpt_header)
            free(gpt_header);
    if (pentries)
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
    uint8_t  *gpt_header = NULL;
    uint32_t crc;
    uint32_t blk_size = 0;

    *state = GPT_OK;

    if (ioctl(fd, BLKSSZGET, &blk_size) != 0) {
            fprintf(stderr, "Failed to get GPT device block size: %s\n",
                            strerror(errno));
            goto error;
    }
    fprintf(stderr, "gpt_get_state: Block size is  %d\n",
                    blk_size);
    gpt_header = (uint8_t*)malloc(blk_size);
    if (!gpt_header) {
            fprintf(stderr, "gpt_get_state:Failed to alloc memory for header\n");
            goto error;
    }
    if (gpt == PRIMARY_GPT)
        gpt_header_offset = blk_size;
    else {
        gpt_header_offset = lseek64(fd, 0, SEEK_END) - blk_size;
        if (gpt_header_offset < 0) {
            fprintf(stderr, "gpt_get_state:Seek to end of GPT part fail\n");
            goto error;
        }
    }

    if (blk_rw(fd, 0, gpt_header_offset, gpt_header, blk_size)) {
        fprintf(stderr, "gpt_get_state: blk_rw failed\n");
        goto error;
    }
    if (memcmp(gpt_header, GPT_SIGNATURE, sizeof(GPT_SIGNATURE)))
        *state = GPT_BAD_SIGNATURE;
    gpt_header_size = GET_4_BYTES(gpt_header + HEADER_SIZE_OFFSET);

    crc = GET_4_BYTES(gpt_header + HEADER_CRC_OFFSET);
    /* header CRC is calculated with this field cleared */
    PUT_4_BYTES(gpt_header + HEADER_CRC_OFFSET, 0);
    if (sparse_crc32(0, gpt_header, gpt_header_size) != crc)
        *state = GPT_BAD_CRC;
    free(gpt_header);
    return 0;
error:
    if (gpt_header)
            free(gpt_header);
    return -1;
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
    uint8_t  *gpt_header = NULL;
    uint32_t crc;
    uint32_t blk_size = 0;

    if (ioctl(fd, BLKSSZGET, &blk_size) != 0) {
            fprintf(stderr, "Failed to get GPT device block size: %s\n",
                            strerror(errno));
            goto error;
    }
    gpt_header = (uint8_t*)malloc(blk_size);
    if (!gpt_header) {
            fprintf(stderr, "Failed to alloc memory for gpt header\n");
            goto error;
    }
    if (gpt == PRIMARY_GPT)
        gpt_header_offset = blk_size;
    else {
        gpt_header_offset = lseek64(fd, 0, SEEK_END) - blk_size;
        if (gpt_header_offset < 0) {
            fprintf(stderr, "Failed to seek to end of GPT device\n");
            goto error;
        }
    }
    if (blk_rw(fd, 0, gpt_header_offset, gpt_header, blk_size)) {
        fprintf(stderr, "Failed to r/w gpt header\n");
        goto error;
    }
    if (state == GPT_OK)
        memcpy(gpt_header, GPT_SIGNATURE, sizeof(GPT_SIGNATURE));
    else if (state == GPT_BAD_SIGNATURE)
        *gpt_header = 0;
    else {
        fprintf(stderr, "gpt_set_state: Invalid state\n");
        goto error;
    }

    gpt_header_size = GET_4_BYTES(gpt_header + HEADER_SIZE_OFFSET);

    /* header CRC is calculated with this field cleared */
    PUT_4_BYTES(gpt_header + HEADER_CRC_OFFSET, 0);
    crc = sparse_crc32(0, gpt_header, gpt_header_size);
    PUT_4_BYTES(gpt_header + HEADER_CRC_OFFSET, crc);

    if (blk_rw(fd, 1, gpt_header_offset, gpt_header, blk_size)) {
        fprintf(stderr, "gpt_set_state: blk write failed\n");
        goto error;
    }
    return 0;
error:
    if(gpt_header)
           free(gpt_header);
    return -1;
}

//dev_path is the path to the block device that contains the GPT image that
//needs to be updated. This would be the device which holds one or more critical
//boot partitions and their backups. In the case of EMMC this function would
//be invoked only once on /dev/block/mmcblk1 since it holds the GPT image
//containing all the partitions For UFS devices it could potentially be
//invoked multiple times, once for each LUN containing critical image(s) and
//their backups
int prepare_partitions(enum boot_update_stage stage, const char *dev_path)
{
    int r;
    int fd = -1;
    enum gpt_state gpt_prim, gpt_second;
    enum boot_update_stage internal_stage;

    if (!dev_path) {
        fprintf(stderr, "Invalid dev_path passed to prepare_partitions\n");
        r = -1;
        goto EXIT;
    }
    fd = open(dev_path, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Opening '%s' failed: %s\n", BLK_DEV_FILE,
                strerror(errno));
        r = -1;
        goto EXIT;
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
    if (fd >= 0) {
       fsync(fd);
       close(fd);
    }
    return r;
}

int add_lun_to_update_list(char *lun_path, struct update_data *dat)
{
        int i = 0;
        struct stat st;
        if (!lun_path || !dat){
                fprintf(stderr, "Invalid data passed to add_lun_to_update_list");
                return -1;
        }
        if (stat(lun_path, &st)) {
                fprintf(stderr, "Unable to access %s. Skipping adding to list",
                                lun_path);
                return -1;
        }
        if (dat->num_valid_entries == 0) {
                fprintf(stderr, "Copying %s into lun_list[%d]\n",
                                lun_path,
                                i);
                strlcpy(dat->lun_list[0], lun_path,
                                MAX_PATH_LEN * sizeof(char));
                dat->num_valid_entries = 1;
        } else {
                for (i = 0; (i < dat->num_valid_entries) &&
                                (dat->num_valid_entries < MAX_LUNS - 1); i++) {
                        //Check if the current LUN is not already part
			//of the lun list
                        if (!strncmp(lun_path,dat->lun_list[i],
                                                strlen(dat->lun_list[i]))) {
                                //LUN already in list..Return
                                return 0;
                        }
                }
                fprintf(stderr, "Copying %s into lun_list[%d]\n",
                                lun_path,
                                dat->num_valid_entries);
                //Add LUN path lun list
                strlcpy(dat->lun_list[dat->num_valid_entries], lun_path,
                                MAX_PATH_LEN * sizeof(char));
                dat->num_valid_entries++;
        }
        return 0;
}

int prepare_boot_update(enum boot_update_stage stage)
{
        int r, fd;
        int is_ufs = 0;
        struct stat ufs_dir_stat;
        struct update_data data;
        int rcode = 0;
        int i = 0;
        int is_error = 0;
        const char ptn_swap_list[][MAX_GPT_NAME_SIZE] = { PTN_SWAP_LIST };
        //Holds /dev/block/bootdevice/by-name/*bak entry
        char buf[MAX_PATH_LEN] = {0};
        //Holds the resolved path of the symlink stored in buf
        char real_path[MAX_PATH_LEN] = {0};

        if(stat(UFS_DEV_DIR, &ufs_dir_stat)) {
                is_ufs = 0;
        } else {
                fprintf(stderr, "UFS device detected\n");
                is_ufs = 1;
        }
        if (!is_ufs) {
                //emmc device. Just pass in path to mmcblk0
                return prepare_partitions(stage, BLK_DEV_FILE);
        } else {
                //Now we need to find the list of LUNs over
                //which the boot critical images are spread
                //and set them up for failsafe updates.To do
                //this we find out where the symlinks for the
                //each of the paths under
                ///dev/block/bootdevice/by-name/PTN_SWAP_LIST
                //actually point to.
                memset(&data, '\0', sizeof(struct update_data));
                for (i=0; i < ARRAY_SIZE(ptn_swap_list); i++) {
                        snprintf(buf, sizeof(buf),
                                        "%s/%sbak",
                                        BOOT_DEV_DIR,
                                        ptn_swap_list[i]);
                        fprintf(stderr, "Attempting to process %s\n", buf);
                        if (stat(buf, &ufs_dir_stat)) {
                                fprintf(stderr, "%s not present. Skipping\n",
                                                buf);
                                continue;
                        }
                        if (readlink(buf, real_path, sizeof(real_path)) < 0)
                        {
                                fprintf(stderr, "readlink error. Skipping %s",
                                                strerror(errno));
                        } else {
                                real_path[PATH_TRUNCATE_LOC] = '\0';
                                add_lun_to_update_list(real_path, &data);
                        }
                        memset(buf, '\0', sizeof(buf));
                        memset(real_path, '\0', sizeof(real_path));
                }
                for (i=0; i < data.num_valid_entries; i++) {
                        fprintf(stderr, "Preparing %s for update stage %d\n",
                                        data.lun_list[i],
                                        stage);
                        rcode = prepare_partitions(stage, data.lun_list[i]);
                        if (rcode != 0)
                        {
                                fprintf(stderr, "Failed to prepare %s.Continuing..\n",
                                                data.lun_list[i]);
                                is_error = 1;
                        }
                }
        }
        if (is_error)
                return -1;
        return 0;
}
