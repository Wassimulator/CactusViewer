/*GIF decoder
===========

This is a small C library that can be used to read GIF files.

Features
--------

  * support for all standard GIF features
  * support for Netscape Application Extension (looping information)
  * other extensions may be easily supported via user hooks
  * small and portable: less than 500 lines of C99
  * public domain


Limitations
-----------

  * no support for GIF files that don't have a global color table
  * no direct support for the plain text extension (rarely used)


Documentation
-------------

0. Essential GIF concepts

GIF  animations  are  stored  in  files as  a  series  of  palette-based
compressed frames.

In order to display the animation, a  program must lay the frames on top
of a  fixed-size canvas,  one after  the other. Each  frame has  a size,
position and  duration. Each  frame can  have its own  palette or  use a
global palette defined in the beginning of the file.

In order to  properly use extension hooks, it's  necessary to understand
how GIF files store variable-sized data. A GIF block of variable size is
a sequence  of sub-blocks. The first  byte in a sub-block  indicates the
number of data bytes to follow. The  end of the block is indicated by an
empty sub-block: one  byte of value 0x00. For instance,  a data block of
600 bytes is stored as 4 sub-blocks:

  255, <255 data bytes>, 255, <255 data bytes>, 90, <90 data bytes>, 0

1. Opening and closing a GIF file

The function `gd_open_gif()` tries to open a GIF file for reading.

    gd_GIF *gd_open_gif(const char *fname);

If this function fails, it returns NULL.

If `gd_open_gif()` succeeds, it returns  a GIF handler (`gd_GIF *`). The
GIF handler  can be passed to  the other gifdec functions  to decode GIF
metadata and frames.

To close  the GIF file  and free memory after  it has been  decoded, the
function `gd_close_gif()` must be called.

    void gd_close_gif(gd_GIF *gif);

2. Reading GIF attributes

Once a GIF file has been successfully opened, some basic information can
be read directly from the GIF handler:

    gd_GIF *gif = gd_open_gif("animation.gif");
    printf("canvas size: %ux%u\n", gif->width, gif->height);
    printf("number of colors: %d\n", gif->palette->size);

3. Reading frames

The function `gd_get_frame()` decodes one frame from the GIF file.

    int gd_get_frame(gd_GIF *gif);

This function returns 0 if there are no more frames to read.

The decoded frame  is stored in `gif->frame`, which is  a buffer of size
`gif->width * gif->height`, in bytes. Each byte value is an index to the
palette at `gif->palette`.

Since GIF files often only store  the rectangular region of a frame that
changed  from the  previous frame,  this function  will only  update the
bytes in `gif->frame`  that are in that region. For  GIF files that only
use  the global  palette, the  whole state  of the  canvas is  stored in
`gif->frame`  at all  times,  in the  form of  an  indexed color  image.
However, when  local palettes are  used, it's  not enough to  keep color
indices from previous frames. The color RGB values themselves need to be
stored.

For this  reason, in order  to get the whole  state of the  canvas after
a  new  frame  has  been  read, it's  necessary  to  call  the  function
`gd_render_frame()`, which writes all pixels to a given buffer.

    void gd_render_frame(gd_GIF *gif, uint8_t *buffer);

The buffer  size must  be at  least `gif->width *  gif->height *  3`, in
bytes. The function `gd_render_frame()` writes  the 24-bit RGB values of
all canvas pixels in it.

4. Frame duration

GIF animations  are not  required to  have a  constant frame  rate. Each
frame can  have a different duration,  which is stored right  before the
frame in a Graphic Control Extension  (GCE) block. This type of block is
read  by gifdec  into a  `gd_GCE` struct  that is  a member  of the  GIF
handler. Specifically,  the unsigned integer `gif->gce.delay`  holds the
current frame duration,  in hundreths of a second. That  means that, for
instance, if  `gif->gce.delay` is `50`,  then the current frame  must be
displayed for half a second.

5. Looping

Most GIF  animations are supposed  to loop automatically, going  back to
the first frame  after the last one is displayed.  GIF files may contain
looping instruction in the form of a non-negative number. If this number
is  zero,  the  animation  must loop  forever.  Otherwise,  this  number
indicates how many times the animation  must be played. When `gifdec` is
decoding a GIF file, this number is stored in `gif->loop_count`.

The function `gd_rewind()` must be called to go back to the start of the
GIF file without closing and reopening it.

    void gd_rewind(gd_GIF *gif);

6. Putting it all together

A simplified skeleton of a GIF viewer may look like this:

    gd_GIF *gif = gd_open_gif("some_animation.gif");
    char *buffer = malloc(gif->width * gif->height * 3);
    for (unsigned looped = 1;; looped++) {
        while (gd_get_frame(gif)) {
            gd_render_frame(gif, buffer);
            // insert code to render buffer to screen
            //    and wait for delay time to pass here
        }
        if (looped == gif->loop_count)
            break;
        gd_rewind(gif);
    }
    free(buffer);
    gd_close_gif(gif);

7. Transparent Background

GIFs can mark a certain color in the palette as the "Background Color".
Pixels having this  color are usually treated as  transparent pixels by
applications.

The function `gd_is_bgcolor()`  can be used to check whether  a pixel in
the canvas currently has background color.

    int gd_is_bgcolor(gd_GIF *gif, uint8_t color[3]);

Here's an example of how to use it:

    gd_render_frame(gif, buffer);
    color = buffer;
    for (y = 0; y < gif->height; y++) {
        for (x = 0; x < gif->width; x++) {
            if (gd_is_bgcolor(gif, color))
                transparent_pixel(x, y);
            else
                opaque_pixel(x, y, color);
            color += 3;
        }
    }

8. Reading streamed metadata with extension hooks

Some  metadata blocks  may occur  any number  of times  in GIF  files in
between frames.  By default, gifdec  ignore these blocks.  However, it's
possible to  setup callback functions  to handle each type  of extension
block, by changing some GIF handler members.

Whenever a Comment Extension block is found, `gif->comment()` is called.

    void (*comment)(struct gd_GIF *gif);

As defined in  the GIF specification, "[t]he  Comment Extension contains
textual information which is not part  of the actual graphics in the GIF
Data Stream." Encoders  are recommended to only include  "text using the
7-bit ASCII character set" in GIF comments.

The actual comment is stored as  a variable-sized block and must be read
from  the file  (using the  file descriptor  `gif->fd`) by  the callback
function. Here's an example, printing the comment to stdout:

    void
    comment(gd_GIF *gif)
    {
        uint8_t sub_len, byte, i;
        do {
            read(gif->fd, &sub_len, 1);
            for (i = 0; i < sub_len; i++) {
                read(gif->fd, &byte, 1);
                printf("%c", byte);
            }
        } while (sub_len);
        printf("\n");
    }

    // ...

    // Somewhere on the main path of execution.
    gif->comment = comment;


Whenever a Plain  Text Extension block is  found, `gif->plain_text()` is
called.

    void (*plain_text)(
        struct gd_GIF *gif, uint16_t tx, uint16_t ty,
        uint16_t tw, uint16_t th, uint8_t cw, uint8_t ch,
        uint8_t fg, uint8_t bg
    );

According to the GIF specification, "[t]he Plain Text Extension contains
textual  data and  the parameters  necessary to  render that  data as  a
graphic  [...]". This  is  a  rarely used  extension  that requires  the
decoder to actually render text on the canvas. In order to support this,
one  must  read the  relevant  specification  and implement  a  suitable
callback function to setup as `gif->plain_text`.

The actual  plain text is stored  as a variable-sized block  and must be
read from the file by the callback function.


Whenever   an   unknown   Application    Extension   block   is   found,
`gif->application()` is called.

    void (*application)(struct gd_GIF *gif, char id[8], char auth[3]);

Application  Extensions  are  used  to  extend  GIF  with  extraofficial
features.  Currently,  gifdec  only  supports  the  so-called  "Netscape
Application  Extension",  which  is  commonly used  to  specify  looping
behavior. Other Application Extensions may be supported via this hook.

The application  data is stored  as a  variable-sized block and  must be
read from the file by the callback function.


Example
-------

The file "example.c" is  a demo GIF player based on  gifdec and SDL2. It
can be tested like this:

    $ cc `pkg-config --cflags --libs sdl2` -o gifplay gifdec.c example.c
    $ ./gifplay animation.gif

That should display the animation. Press SPACE to pause and Q to quit.

Copying
-------

All of the source code and documentation for gifdec is released into the
public domain and provided without warranty of any kind.
*/

#pragma once
#ifndef GIFDEC_H
#define GIFDEC_H

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct gd_Palette
    {
        int size;
        uint8_t colors[0x100 * 3];
    } gd_Palette;

    typedef struct gd_GCE
    {
        uint16_t delay;
        uint8_t tindex;
        uint8_t disposal;
        int input;
        int transparency;
    } gd_GCE;

    typedef struct gd_GIF
    {
        int fd;
        off_t anim_start;
        uint16_t width, height;
        uint16_t depth;
        uint16_t loop_count;
        gd_GCE gce;
        gd_Palette *palette;
        gd_Palette lct, gct;
        void (*plain_text)(
            struct gd_GIF *gif, uint16_t tx, uint16_t ty,
            uint16_t tw, uint16_t th, uint8_t cw, uint8_t ch,
            uint8_t fg, uint8_t bg);
        void (*comment)(struct gd_GIF *gif);
        void (*application)(struct gd_GIF *gif, char id[8], char auth[3]);
        uint16_t fx, fy, fw, fh;
        uint8_t bgindex;
        uint8_t *canvas, *frame;
    } gd_GIF;

    gd_GIF *gd_open_gif(const char *fname);
    int gd_get_frame(gd_GIF *gif);
    void gd_render_frame(gd_GIF *gif, uint8_t *buffer);
    int gd_is_bgcolor(gd_GIF *gif, uint8_t color[3]);
    void gd_rewind(gd_GIF *gif);
    void gd_close_gif(gd_GIF *gif);

#ifdef __cplusplus
}
#endif

#endif /* GIFDEC_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

typedef struct Entry
{
    uint16_t length;
    uint16_t prefix;
    uint8_t suffix;
} Entry;

typedef struct Table
{
    int bulk;
    int nentries;
    Entry *entries;
} Table;

static uint16_t
read_num(int fd)
{
    uint8_t bytes[2];

    read(fd, bytes, 2);
    return bytes[0] + (((uint16_t)bytes[1]) << 8);
}

gd_GIF *
gd_open_gif(const char *fname)
{
    int fd;
    uint8_t sigver[3];
    uint16_t width, height, depth;
    uint8_t fdsz, bgidx, aspect;
    int i;
    uint8_t *bgcolor;
    int gct_sz;
    gd_GIF *gif;

    fd = open(fname, O_RDONLY);
    if (fd == -1)
        return NULL;
#ifdef _WIN32
    setmode(fd, O_BINARY);
#endif
    /* Header */
    read(fd, sigver, 3);
    if (memcmp(sigver, "GIF", 3) != 0)
    {
        fprintf(stderr, "invalid signature\n");
        goto fail;
    }
    /* Version */
    read(fd, sigver, 3);
    if (memcmp(sigver, "89a", 3) != 0)
    {
        fprintf(stderr, "invalid version\n");
        goto fail;
    }
    /* Width x Height */
    width = read_num(fd);
    height = read_num(fd);
    /* FDSZ */
    read(fd, &fdsz, 1);
    /* Presence of GCT */
    if (!(fdsz & 0x80))
    {
        fprintf(stderr, "no global color table\n");
        goto fail;
    }
    /* Color Space's Depth */
    depth = ((fdsz >> 4) & 7) + 1;
    /* Ignore Sort Flag. */
    /* GCT Size */
    gct_sz = 1 << ((fdsz & 0x07) + 1);
    /* Background Color Index */
    read(fd, &bgidx, 1);
    /* Aspect Ratio */
    read(fd, &aspect, 1);
    /* Create gd_GIF Structure. */
    gif = (gd_GIF *)calloc(1, sizeof(*gif));
    if (!gif)
        goto fail;
    gif->fd = fd;
    gif->width = width;
    gif->height = height;
    gif->depth = depth;
    /* Read GCT */
    gif->gct.size = gct_sz;
    read(fd, gif->gct.colors, 3 * gif->gct.size);
    gif->palette = &gif->gct;
    gif->bgindex = bgidx;
    gif->frame = (uint8_t *)calloc(4, width * height);
    if (!gif->frame)
    {
        free(gif);
        goto fail;
    }
    gif->canvas = &gif->frame[width * height];
    if (gif->bgindex)
        memset(gif->frame, gif->bgindex, gif->width * gif->height);
    bgcolor = &gif->palette->colors[gif->bgindex * 3];
    if (bgcolor[0] || bgcolor[1] || bgcolor[2])
        for (i = 0; i < gif->width * gif->height; i++)
            memcpy(&gif->canvas[i * 3], bgcolor, 3);
    gif->anim_start = lseek(fd, 0, SEEK_CUR);
    goto ok;
fail:
    close(fd);
    return 0;
ok:
    return gif;
}

static void
discard_sub_blocks(gd_GIF *gif)
{
    uint8_t size;

    do
    {
        read(gif->fd, &size, 1);
        lseek(gif->fd, size, SEEK_CUR);
    } while (size);
}

static void
read_plain_text_ext(gd_GIF *gif)
{
    if (gif->plain_text)
    {
        uint16_t tx, ty, tw, th;
        uint8_t cw, ch, fg, bg;
        off_t sub_block;
        lseek(gif->fd, 1, SEEK_CUR); /* block size = 12 */
        tx = read_num(gif->fd);
        ty = read_num(gif->fd);
        tw = read_num(gif->fd);
        th = read_num(gif->fd);
        read(gif->fd, &cw, 1);
        read(gif->fd, &ch, 1);
        read(gif->fd, &fg, 1);
        read(gif->fd, &bg, 1);
        sub_block = lseek(gif->fd, 0, SEEK_CUR);
        gif->plain_text(gif, tx, ty, tw, th, cw, ch, fg, bg);
        lseek(gif->fd, sub_block, SEEK_SET);
    }
    else
    {
        /* Discard plain text metadata. */
        lseek(gif->fd, 13, SEEK_CUR);
    }
    /* Discard plain text sub-blocks. */
    discard_sub_blocks(gif);
}

static void
read_graphic_control_ext(gd_GIF *gif)
{
    uint8_t rdit;

    /* Discard block size (always 0x04). */
    lseek(gif->fd, 1, SEEK_CUR);
    read(gif->fd, &rdit, 1);
    gif->gce.disposal = (rdit >> 2) & 3;
    gif->gce.input = rdit & 2;
    gif->gce.transparency = rdit & 1;
    gif->gce.delay = read_num(gif->fd);
    read(gif->fd, &gif->gce.tindex, 1);
    /* Skip block terminator. */
    lseek(gif->fd, 1, SEEK_CUR);
}

static void
read_comment_ext(gd_GIF *gif)
{
    if (gif->comment)
    {
        off_t sub_block = lseek(gif->fd, 0, SEEK_CUR);
        gif->comment(gif);
        lseek(gif->fd, sub_block, SEEK_SET);
    }
    /* Discard comment sub-blocks. */
    discard_sub_blocks(gif);
}

static void
read_application_ext(gd_GIF *gif)
{
    char app_id[8];
    char app_auth_code[3];

    /* Discard block size (always 0x0B). */
    lseek(gif->fd, 1, SEEK_CUR);
    /* Application Identifier. */
    read(gif->fd, app_id, 8);
    /* Application Authentication Code. */
    read(gif->fd, app_auth_code, 3);
    if (!strncmp(app_id, "NETSCAPE", sizeof(app_id)))
    {
        /* Discard block size (0x03) and constant byte (0x01). */
        lseek(gif->fd, 2, SEEK_CUR);
        gif->loop_count = read_num(gif->fd);
        /* Skip block terminator. */
        lseek(gif->fd, 1, SEEK_CUR);
    }
    else if (gif->application)
    {
        off_t sub_block = lseek(gif->fd, 0, SEEK_CUR);
        gif->application(gif, app_id, app_auth_code);
        lseek(gif->fd, sub_block, SEEK_SET);
        discard_sub_blocks(gif);
    }
    else
    {
        discard_sub_blocks(gif);
    }
}

static void
read_ext(gd_GIF *gif)
{
    uint8_t label;

    read(gif->fd, &label, 1);
    switch (label)
    {
    case 0x01:
        read_plain_text_ext(gif);
        break;
    case 0xF9:
        read_graphic_control_ext(gif);
        break;
    case 0xFE:
        read_comment_ext(gif);
        break;
    case 0xFF:
        read_application_ext(gif);
        break;
    default:
        fprintf(stderr, "unknown extension: %02X\n", label);
    }
}

static Table *
new_table(int key_size)
{
    int key;
    int init_bulk = MAX(1 << (key_size + 1), 0x100);
    Table *table = (Table *)malloc(sizeof(*table) + sizeof(Entry) * init_bulk);
    if (table)
    {
        table->bulk = init_bulk;
        table->nentries = (1 << key_size) + 2;
        table->entries = (Entry *)&table[1];
        for (key = 0; key < (1 << key_size); key++)
        {
            table->entries[key].length = 1;
            table->entries[key].prefix = 0xFFF;
            table->entries[key].suffix = key;
        }
    }
    return table;
}

/* Add table entry. Return value:
 *  0 on success
 *  +1 if key size must be incremented after this addition
 *  -1 if could not realloc table */
static int
add_entry(Table **tablep, uint16_t length, uint16_t prefix, uint8_t suffix)
{
    Table *table = *tablep;
    if (table->nentries == table->bulk)
    {
        table->bulk *= 2;
        table = (Table *)realloc(table, sizeof(*table) + sizeof(Entry) * table->bulk);
        if (!table)
            return -1;
        table->entries = (Entry *)&table[1];
        *tablep = table;
    }
    table->entries[table->nentries].length = length;
    table->entries[table->nentries].prefix = prefix;
    table->entries[table->nentries].suffix = suffix;
    table->nentries++;
    if ((table->nentries & (table->nentries - 1)) == 0)
        return 1;
    return 0;
}

static uint16_t
get_key(gd_GIF *gif, int key_size, uint8_t *sub_len, uint8_t *shift, uint8_t *byte)
{
    int bits_read;
    int rpad;
    int frag_size;
    uint16_t key;

    key = 0;
    for (bits_read = 0; bits_read < key_size; bits_read += frag_size)
    {
        rpad = (*shift + bits_read) % 8;
        if (rpad == 0)
        {
            /* Update byte. */
            if (*sub_len == 0)
            {
                read(gif->fd, sub_len, 1); /* Must be nonzero! */
                if (*sub_len == 0)
                    return 0x1000;
            }
            read(gif->fd, byte, 1);
            (*sub_len)--;
        }
        frag_size = MIN(key_size - bits_read, 8 - rpad);
        key |= ((uint16_t)((*byte) >> rpad)) << bits_read;
    }
    /* Clear extra bits to the left. */
    key &= (1 << key_size) - 1;
    *shift = (*shift + key_size) % 8;
    return key;
}

/* Compute output index of y-th input line, in frame of height h. */
static int
interlaced_line_index(int h, int y)
{
    int p; /* number of lines in current pass */

    p = (h - 1) / 8 + 1;
    if (y < p) /* pass 1 */
        return y * 8;
    y -= p;
    p = (h - 5) / 8 + 1;
    if (y < p) /* pass 2 */
        return y * 8 + 4;
    y -= p;
    p = (h - 3) / 4 + 1;
    if (y < p) /* pass 3 */
        return y * 4 + 2;
    y -= p;
    /* pass 4 */
    return y * 2 + 1;
}

/* Decompress image pixels.
 * Return 0 on success or -1 on out-of-memory (w.r.t. LZW code table). */
static int
read_image_data(gd_GIF *gif, int interlace)
{
    uint8_t sub_len, shift, byte;
    int init_key_size, key_size, table_is_full;
    int frm_off, frm_size, str_len, i, p, x, y;
    uint16_t key, clear, stop;
    int ret;
    Table *table;
    Entry entry;
    off_t start, end;

    read(gif->fd, &byte, 1);
    key_size = (int)byte;
    if (key_size < 2 || key_size > 8)
        return -1;

    start = lseek(gif->fd, 0, SEEK_CUR);
    discard_sub_blocks(gif);
    end = lseek(gif->fd, 0, SEEK_CUR);
    lseek(gif->fd, start, SEEK_SET);
    clear = 1 << key_size;
    stop = clear + 1;
    table = new_table(key_size);
    key_size++;
    init_key_size = key_size;
    sub_len = shift = 0;
    key = get_key(gif, key_size, &sub_len, &shift, &byte); /* clear code */
    frm_off = 0;
    ret = 0;
    frm_size = gif->fw * gif->fh;
    while (frm_off < frm_size)
    {
        if (key == clear)
        {
            key_size = init_key_size;
            table->nentries = (1 << (key_size - 1)) + 2;
            table_is_full = 0;
        }
        else if (!table_is_full)
        {
            ret = add_entry(&table, str_len + 1, key, entry.suffix);
            if (ret == -1)
            {
                free(table);
                return -1;
            }
            if (table->nentries == 0x1000)
            {
                ret = 0;
                table_is_full = 1;
            }
        }
        key = get_key(gif, key_size, &sub_len, &shift, &byte);
        if (key == clear)
            continue;
        if (key == stop || key == 0x1000)
            break;
        if (ret == 1)
            key_size++;
        entry = table->entries[key];
        str_len = entry.length;
        for (i = 0; i < str_len; i++)
        {
            p = frm_off + entry.length - 1;
            x = p % gif->fw;
            y = p / gif->fw;
            if (interlace)
                y = interlaced_line_index((int)gif->fh, y);
            gif->frame[(gif->fy + y) * gif->width + gif->fx + x] = entry.suffix;
            if (entry.prefix == 0xFFF)
                break;
            else
                entry = table->entries[entry.prefix];
        }
        frm_off += str_len;
        if (key < table->nentries - 1 && !table_is_full)
            table->entries[table->nentries - 1].suffix = entry.suffix;
    }
    free(table);
    if (key == stop)
        read(gif->fd, &sub_len, 1); /* Must be zero! */
    lseek(gif->fd, end, SEEK_SET);
    return 0;
}

/* Read image.
 * Return 0 on success or -1 on out-of-memory (w.r.t. LZW code table). */
static int
read_image(gd_GIF *gif)
{
    uint8_t fisrz;
    int interlace;

    /* Image Descriptor. */
    gif->fx = read_num(gif->fd);
    gif->fy = read_num(gif->fd);

    if (gif->fx >= gif->width || gif->fy >= gif->height)
        return -1;

    gif->fw = read_num(gif->fd);
    gif->fh = read_num(gif->fd);

    gif->fw = MIN(gif->fw, gif->width - gif->fx);
    gif->fh = MIN(gif->fh, gif->height - gif->fy);

    read(gif->fd, &fisrz, 1);
    interlace = fisrz & 0x40;
    /* Ignore Sort Flag. */
    /* Local Color Table? */
    if (fisrz & 0x80)
    {
        /* Read LCT */
        gif->lct.size = 1 << ((fisrz & 0x07) + 1);
        read(gif->fd, gif->lct.colors, 3 * gif->lct.size);
        gif->palette = &gif->lct;
    }
    else
        gif->palette = &gif->gct;
    /* Image Data. */
    return read_image_data(gif, interlace);
}

static void
render_frame_rect(gd_GIF *gif, uint8_t *buffer)
{
    int i, j, k;
    uint8_t index, *color;
    i = gif->fy * gif->width + gif->fx;
    for (j = 0; j < gif->fh; j++)
    {
        for (k = 0; k < gif->fw; k++)
        {
            index = gif->frame[(gif->fy + j) * gif->width + gif->fx + k];
            color = &gif->palette->colors[index * 3];
            if (!gif->gce.transparency || index != gif->gce.tindex)
                memcpy(&buffer[(i + k) * 3], color, 3);
        }
        i += gif->width;
    }
}

static void
dispose(gd_GIF *gif)
{
    int i, j, k;
    uint8_t *bgcolor;
    switch (gif->gce.disposal)
    {
    case 2: /* Restore to background color. */
        bgcolor = &gif->palette->colors[gif->bgindex * 3];
        i = gif->fy * gif->width + gif->fx;
        for (j = 0; j < gif->fh; j++)
        {
            for (k = 0; k < gif->fw; k++)
                memcpy(&gif->canvas[(i + k) * 3], bgcolor, 3);
            i += gif->width;
        }
        break;
    case 3: /* Restore to previous, i.e., don't update canvas.*/
        break;
    default:
        /* Add frame non-transparent pixels to canvas. */
        render_frame_rect(gif, gif->canvas);
    }
}

/* Return 1 if got a frame; 0 if got GIF trailer; -1 if error. */
int gd_get_frame(gd_GIF *gif)
{
    char sep;

    dispose(gif);
    read(gif->fd, &sep, 1);
    while (sep != ',')
    {
        if (sep == ';')
            return 0;
        if (sep == '!')
            read_ext(gif);
        else
            return -1;
        read(gif->fd, &sep, 1);
    }
    if (read_image(gif) == -1)
        return -1;
    return 1;
}

void gd_render_frame(gd_GIF *gif, uint8_t *buffer)
{
    memcpy(buffer, gif->canvas, gif->width * gif->height * 3);
    render_frame_rect(gif, buffer);
}

int gd_is_bgcolor(gd_GIF *gif, uint8_t color[3])
{
    return !memcmp(&gif->palette->colors[gif->bgindex * 3], color, 3);
}

void gd_rewind(gd_GIF *gif)
{
    lseek(gif->fd, gif->anim_start, SEEK_SET);
}

void gd_close_gif(gd_GIF *gif)
{
    close(gif->fd);
    free(gif->frame);
    free(gif);
}
