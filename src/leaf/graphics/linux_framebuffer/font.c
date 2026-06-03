#ifdef GRAPHICS_LINUX_FBDEV

#include <zlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "../../font.h"


// References:
// - https://aeb.win.tue.nl/linux/kbd/font-formats-1.html
// - https://wiki.osdev.org/PC_Screen_Font
// - https://www.zap.org.au/projects/console-fonts-utils/

#define PSF1_MODE512 0x01


bool leaf_load_font(LeafFont* font, const char* filepath) {
    font->loaded = false;

    gzFile file = gzopen(filepath, "r");
    if(file == NULL) {

        fprintf(stderr, "%s: %s(): Failed to open font file '%s'\n",
                __FILE__, __func__, filepath);
        goto out;
    }


    // Read the header into psf2 first
    // Because we dont yet know the version.

    PSF2_Header header = (PSF2_Header){0};
    z_size_t header_read_len 
        = gzfread((void*)&header, sizeof header, 1, file);

    printf("%s(): (debug) read header: %li\n",
            __func__, header_read_len);

    if(gzeof(file)) {
        fprintf(stderr, "%s: %s(): Could not read header of '%s'\n",
                __FILE__, __func__, filepath);
        fprintf(stderr, "gzerror: %s", gzerror(file, NULL));
        goto close_n_out;
    }


    memcpy(&font->psf_header, &header, sizeof font->psf_header);
    font->psf_data = NULL;

    if(header.magic_bytes[0] == 0x36
    && header.magic_bytes[1] == 0x04
    ){
        printf("%s(): (debug) %s <- PSF1\n", __func__, filepath);
        font->psf_version = PSF_VERSION_1;
        font->psf_data_size 
            = ((font->psf_header.psf1.mode & PSF1_MODE512) ? 512 : 256)
            * font->psf_header.psf1.char_size;

        font->real_char_width  = 8;
        font->real_char_height = font->psf_header.psf1.char_size;
    }
    else
    if(header.magic_bytes[0] == 0x72
    && header.magic_bytes[1] == 0xb5
    && header.magic_bytes[2] == 0x4a
    && header.magic_bytes[3] == 0x86
    ){
        printf("%s(): (debug) %s <- PSF2\n", __func__, filepath);
        font->psf_version = PSF_VERSION_2;
        font->psf_data_size 
            = font->psf_header.psf2.length * font->psf_header.psf2.height;
    
        font->real_char_width  = font->psf_header.psf2.width;
        font->real_char_height = font->psf_header.psf2.height;
    }

    font->psf_data = calloc(font->psf_data_size, sizeof *font->psf_data);

    z_size_t data_read_bytes 
        = gzfread(font->psf_data, font->psf_data_size, 1, file);

    printf("%s(): (debug) read: %li (%s)\n", 
            __func__, data_read_bytes, filepath);
 
    // Set default values.

    font->loaded = true;
    font->italic = 0;
    font->char_width = 0;
    font->char_height = 0;
    leaf_set_font_scale(font, 4.0f);
    leaf_set_font_color(font, (RGBColor){ 255, 255, 255 });
    leaf_set_font_spacing(font, 4.0f);
    leaf_set_font_space_width(font, 8);
    leaf_set_font_tab_width(font, 8*4);


close_n_out:
    gzclose(file);
out:
    return font->loaded;
}

void leaf_unload_font(LeafFont* font) {
}


#endif // GRAPHICS_LINUX_FBDEV
