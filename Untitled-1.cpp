#include "gif_load.h"
#include <fcntl.h>
#ifdef _MSC_VER
/** MSVC is definitely not my favourite compiler...   >_<   **/
#pragma warning(disable : 4996)
#include <io.h>
#else
#include <unistd.h>
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif

#pragma pack(push, 1)
typedef struct
{
    void *data, *pict, *prev;
    unsigned long size, last;
    int uuid;
} STAT; /** #pragma avoids -Wpadded on 64-bit machines **/
#pragma pack(pop)

void Frame(void *, struct GIF_WHDR *); /** keeps -Wmissing-prototypes happy **/
void Frame(void *data, struct GIF_WHDR *whdr)
{
    uint32_t *pict, *prev, x, y, yoff, iter, ifin, dsrc, ddst;
    uint8_t head[18] = {0};
    STAT *stat = (STAT *)data;

#define BGRA(i) ((whdr->bptr[i] == whdr->tran) ? 0 : ((uint32_t)(whdr->cpal[whdr->bptr[i]].R << ((GIF_BIGE) ? 8 : 16)) | (uint32_t)(whdr->cpal[whdr->bptr[i]].G << ((GIF_BIGE) ? 16 : 8)) | (uint32_t)(whdr->cpal[whdr->bptr[i]].B << ((GIF_BIGE) ? 24 : 0)) | ((GIF_BIGE) ? 0xFF : 0xFF000000)))
    if (!whdr->ifrm)
    {
        /** TGA doesn`t support heights over 0xFFFF, so we have to trim: **/
        whdr->nfrm = ((whdr->nfrm < 0) ? -whdr->nfrm : whdr->nfrm) * whdr->ydim;
        whdr->nfrm = (whdr->nfrm < 0xFFFF) ? whdr->nfrm : 0xFFFF;
        /** this is the very first frame, so we must write the header **/
        head[2] = 2;
        head[12] = (uint8_t)(whdr->xdim);
        head[13] = (uint8_t)(whdr->xdim >> 8);
        head[14] = (uint8_t)(whdr->nfrm);
        head[15] = (uint8_t)(whdr->nfrm >> 8);
        head[16] = 32;   /** 32 bits depth **/
        head[17] = 0x20; /** top-down flag **/
        write(stat->uuid, head, 18UL);
        ddst = (uint32_t)(whdr->xdim * whdr->ydim);
        stat->pict = calloc(sizeof(uint32_t), ddst);
        stat->prev = calloc(sizeof(uint32_t), ddst);
    }
    /** [TODO:] the frame is assumed to be inside global bounds,
                however it might exceed them in some GIFs; fix me. **/
    pict = (uint32_t *)stat->pict;
    ddst = (uint32_t)(whdr->xdim * whdr->fryo + whdr->frxo);
    ifin = (!(iter = (whdr->intr) ? 0 : 4)) ? 4 : 5; /** interlacing support **/
    for (dsrc = (uint32_t)-1; iter < ifin; iter++)
        for (yoff = 16U >> ((iter > 1) ? iter : 1), y = (8 >> iter) & 7;
             y < (uint32_t)whdr->fryd; y += yoff)
            for (x = 0; x < (uint32_t)whdr->frxd; x++)
                if (whdr->tran != (long)whdr->bptr[++dsrc])
                    pict[(uint32_t)whdr->xdim * y + x + ddst] = BGRA(dsrc);
    write(stat->uuid, pict, sizeof(uint32_t) * (uint32_t)whdr->xdim * (uint32_t)whdr->ydim);
    if ((whdr->mode == GIF_PREV) && !stat->last)
    {
        whdr->frxd = whdr->xdim;
        whdr->fryd = whdr->ydim;
        whdr->mode = GIF_BKGD;
        ddst = 0;
    }
    else
    {
        stat->last = (whdr->mode == GIF_PREV) ? stat->last : (unsigned long)(whdr->ifrm + 1);
        pict = (uint32_t *)((whdr->mode == GIF_PREV) ? stat->pict : stat->prev);
        prev = (uint32_t *)((whdr->mode == GIF_PREV) ? stat->prev : stat->pict);
        for (x = (uint32_t)(whdr->xdim * whdr->ydim); --x;
             pict[x - 1] = prev[x - 1])
            ;
    }
    if (whdr->mode == GIF_BKGD) /** cutting a hole for the next frame **/
        for (whdr->bptr[0] = (uint8_t)((whdr->tran >= 0) ? whdr->tran : whdr->bkgd), y = 0,
            pict = (uint32_t *)stat->pict;
             y < (uint32_t)whdr->fryd; y++)
            for (x = 0; x < (uint32_t)whdr->frxd; x++)
                pict[(uint32_t)whdr->xdim * y + x + ddst] = BGRA(0);
#undef BGRA
}

int main(int argc, char *argv[])
{
    STAT stat = {0};

    if (argc < 3)
        write(1, "arguments: <in>.gif <out>.tga (1 or more times)\n", 48UL);
    for (stat.uuid = 2, argc -= (~argc & 1); argc >= 3; argc -= 2)
    {
        if ((stat.uuid = open(argv[argc - 2], O_RDONLY | O_BINARY)) <= 0)
            return 1;
        stat.size = (unsigned long)lseek(stat.uuid, 0UL, 2 /** SEEK_END **/);
        lseek(stat.uuid, 0UL, 0 /** SEEK_SET **/);
        read(stat.uuid, stat.data = realloc(0, stat.size), stat.size);
        close(stat.uuid);
        unlink(argv[argc - 1]);
        stat.uuid = open(argv[argc - 1], O_CREAT | O_WRONLY | O_BINARY, 0644);
        if (stat.uuid > 0)
        {
            GIF_Load(stat.data, (long)stat.size, Frame, 0, (void *)&stat, 0L);
            stat.pict = realloc(stat.pict, 0L);
            stat.prev = realloc(stat.prev, 0L);
            close(stat.uuid);
            stat.uuid = 0;
        }
        stat.data = realloc(stat.data, 0L);
    }
    return stat.uuid;
}