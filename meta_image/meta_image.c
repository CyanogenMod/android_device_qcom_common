/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of The Linux Foundation nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#define VERBOSE 1

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <sys/stat.h>
#include "meta_format.h"

#define MAX_IMAGES 16

meta_header_t meta_header;
img_header_entry_t  img_header[MAX_IMAGES];

enum MetaImage
{
    undefined,
    bootloader,
    radio
};

enum MetaImage meta_image = undefined;
struct device_info di;

void usage()
{
    printf("\nUsage:\n");
    printf("meta_image <ptn1> <file1> <ptn2> <file2> ... <ptnN> <fileN> -o <output_file> -v <version>\n");
    printf("eg: meta_image sbl1 sbl1.mbn rpm rpm.mbn tz tz.mbn hyp hyp.mbn aboot emmc_appsboot.mbn -o bootloader.img -v M8916AAAAAULGD21210017.1\n");
}

int read_devinfo ()
{
    FILE *fin = fopen("devinfo.bin", "rb");
    fread(&di, sizeof(di), 1, fin);
    fclose(fin);
}

int write_devinfo ()
{
    FILE *fout;
    /* If image exists, just update it */
    read_devinfo();

    /* No devinfo.bin, create new one */
    strncpy(di.magic, DEVICE_MAGIC, sizeof(DEVICE_MAGIC));
    di.is_unlocked = 1;
    di.is_tampered = 0;
    di.is_verified = 0;
    di.charger_screen_enabled = 1;
    memset(di.display_panel, 0, sizeof(di.display_panel));

    if (meta_image == bootloader)
      memcpy(di.bootloader_version, meta_header.img_version, sizeof(di.bootloader_version));
    else if (meta_image == radio)
      memcpy(di.radio_version, meta_header.img_version, sizeof(di.radio_version));

    fout = fopen("devinfo.bin", "wb");
    fwrite(&di, sizeof(di), 1, fout);
    fclose(fout);
}

int extract_images (char *file_path)
{
    FILE *fp;
    FILE *fout;
    void *buffer;
    int i, images = 0;

#if VERBOSE
    printf ("Extracting images...\n");
#endif

    if(strstr(file_path, "bootloader"))
        meta_image = bootloader;
    else
        meta_image = radio;

    if(file_path != NULL) {
        fp = fopen(file_path, "rb");
        fread(&meta_header, sizeof(meta_header), 1, fp);
        fread(&img_header, sizeof(img_header), 1, fp);
#if VERBOSE
        printf("Image version: %s\n", meta_header.img_version);
#endif

        images = meta_header.img_hdr_sz / sizeof(img_header_entry_t);
        for(i=0; i<images; i++) {
            if((img_header[i].ptn_name == NULL) ||
                (img_header[i].start_offset == 0) ||
                (img_header[i].size == 0))
               break;
#if VERBOSE
            printf("%16s %8d %8d\n", img_header[i].ptn_name, img_header[i].start_offset, img_header[i].size);
#endif
            fout = fopen(strcat(img_header[i].ptn_name,".bin"), "wb");
            fseek(fp, img_header[i].start_offset, SEEK_SET);
            buffer = malloc(img_header[i].size);
            fread(buffer, img_header[i].size, 1, fp);
            fwrite(buffer, img_header[i].size, 1, fout);
            fclose(fout);
        }
        fclose(fp);
    }

    write_devinfo();

    return 0;
}

int main(int argc, char *argv[])
{
    int i, entry=0;
    int images=0;
    int start_offset = sizeof(meta_header_t) + sizeof(img_header);
    struct stat st;
    FILE *f[MAX_IMAGES], *meta_image = NULL;
    void *buffer;

    meta_header.magic = META_HEADER_MAGIC;
    meta_header.major_version = 1;
    meta_header.minor_version = 0;
    meta_header.meta_hdr_sz = sizeof(meta_header);
    meta_header.img_hdr_sz = sizeof(img_header);

    for (i=1; i<argc; i++) {

        if (strcmp(argv[i], "-o") == 0)  {
            /* Output file name */
            meta_image = fopen(argv[++i], "wb");
        } else if (strcmp(argv[i], "-v") == 0)  {
            /* Image version string */
            strncpy(meta_header.img_version, argv[++i], 32);
        } else if (strcmp(argv[i], "-x") == 0)  {
            /* Special case - extract images from meta_image */
            extract_images(argv[++i]);
            return 0;
        } else {
            /* Populate entry in img_header for ptn name, size & offset */
            strcpy(img_header[entry].ptn_name, argv[i]);
            img_header[entry].start_offset = start_offset;
            i++;
            stat(argv[i], &st);
            img_header[entry].size = st.st_size;
            start_offset += st.st_size;
            f[entry] = fopen(argv[i], "rb");
            entry++;
            images++;
        }
    }

    if(meta_image == NULL) {
        printf("Error: output file not specificed\n");
        usage();
        exit(-1);
    }

#if VERBOSE
    printf ("Generating meta image from %d images...\n", images);
#endif
    fwrite (&meta_header, sizeof(meta_header), 1, meta_image);
    fwrite (img_header, sizeof(img_header), 1, meta_image);
#if VERBOSE
    printf ("        ptn_name start_offset  size\n");
#endif
    for (i=0; i<images; i++) {
        printf ("%16s %8d %8d\n", img_header[i].ptn_name,
                                  img_header[i].start_offset,
                                  img_header[i].size);

        buffer = malloc(img_header[i].size);
        fread(buffer, img_header[i].size, 1, f[i]);
        fwrite (buffer, img_header[i].size, 1, meta_image);
        fclose(f[i]);
        free(buffer);
    }

#if VERBOSE
    printf ("Done.\n");
#endif
    fclose(meta_image);
    return 0;
}

