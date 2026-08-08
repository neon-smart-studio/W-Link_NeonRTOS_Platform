#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "NeonRTOS.h"

#include "NeonRtFs.h"

#ifdef NEONRTFS_HEATSHRINK
#include "heatshrink_config_custom.h"
#include "heatshrink_decoder.h"
#endif

#define min(a,b) ((a)<(b)?(a):(b))  /**< Find the minimum of 2 numbers. */

bool NeonRtFsInitDone = false;
uint32_t NeonRtFsData_StartAddr = 0;

NeonRTOS_LockObj_t Flash_Access_MutexHandle = NULL;

extern unsigned char neonrtfs_img[];
extern unsigned int neonrtfs_img_len;
/*
Available locations, at least in my flash, with boundaries partially guessed. This
is using 0.9.1/0.9.2 SDK on a not-too-new module.
0x00000 (0x10000): Code/data (RAM data?)
0x10000 (0x02000): Gets erased by something?
0x12000 (0x2E000): Free (filled with zeroes) (parts used by ESPCloud and maybe SSL)
0x40000 (0x20000): Code/data (ROM data?)
0x60000 (0x1C000): Free
0x7c000 (0x04000): Param store
0x80000 - end of flash

Accessing the flash through the mem emulation at 0x40200000 is a bit hairy: All accesses
*must* be aligned 32-bit accesses. Reading a short, byte or unaligned word will result in
a memory exception, crashing the program.
*/
NeonRtFsInitResult NeonRtFsInit(void) {
	NeonRtFsInitDone = false;

	if (neonrtfs_img_len < sizeof(NeonRtFsHeader))
	{
		return NEONRTFS_INIT_RESULT_NO_IMAGE;
	}
	/*
	if (((int)neonrtfs_img & 3) != 0) {
		return NEONRTFS_INIT_RESULT_BAD_ALIGN;
	}
*/
	// check if there is valid header at address
	NeonRtFsHeader testHeader;
	memcpy(&testHeader, neonrtfs_img, sizeof(NeonRtFsHeader));
        
	if (testHeader.magic != NEONRTFS_MAGIC) {
		return NEONRTFS_INIT_RESULT_NO_IMAGE;
	}
        
	NeonRtFsInitDone = true;
    
	return NEONRTFS_INIT_RESULT_OK;
}

//Copies len bytes over from dst to src, but does it using *only*
//aligned 32-bit reads. Yes, it's no too optimized but it's short and sweet and it works.

//ToDo: perhaps memcpy also does unaligned accesses?
static void readFlashUnaligned(void *dst, const uint8_t* src, size_t len) {
	if(dst==NULL || src==NULL){
#if NEONRTFS_DEBUG==1
		UART_Printf("[NeonRTFS] Invalid Read Buf Pointer!\n");
#endif
		return;
	}
	if(len==0U){
		return;
	}
  
	memcpy(dst, src, len);
}

// Returns flags of opened file.
int NeonRtFsFlags(NeonRtFsFile *fh) {
	if (fh == NULL) {
#if NEONRTFS_DEBUG==1
		UART_Printf("[NeonRTFS] File handle not ready\n");
#endif
		return -1;
	}

	return (int)fh->header.flags;
}

//Open a file and return a pointer to the file desc struct.
NeonRtFsFile *NeonRtFsOpen(const char *fileName) {
	if (NeonRtFsInitDone == false) {
#if NEONRTFS_DEBUG==1
		UART_Printf("[NeonRTFS] Call NeonRtFsInit first!\n");
#endif
		return NULL;
	}
	if (fileName == NULL)
	{
		return NULL;
	}

	const uint8_t* s = neonrtfs_img;
	const uint8_t* image_end = neonrtfs_img + neonrtfs_img_len;
	const uint8_t* p = s;
	char namebuf[256+1];
	NeonRtFsHeader h;
	NeonRtFsFile *r;
	//Strip initial slashes
	while(fileName[0]=='/') fileName++;
	if (fileName[0] == '\0')
	{
		return NULL;
	}
	//Go find that file!
	while(1) {
		//Grab the next file header.
		if ((size_t)(image_end - p) < sizeof(NeonRtFsHeader))
		{
			NeonRtFsInitDone = false;
			return NULL;
		}
		memcpy(&h, p, sizeof(NeonRtFsHeader));
		//Grab the name of the file.
		p+=sizeof(NeonRtFsHeader); 

		if (h.magic!=NEONRTFS_MAGIC) {
#if NEONRTFS_DEBUG==1
			UART_Printf("[NeonRTFS] Magic mismatch. NeonRTFS image broken.\n");
#endif
                        NeonRtFsInitDone = false;
                        
			return NULL;
		}
		if (h.flags&FLAG_LASTFILE) {
#if NEONRTFS_DEBUG==1
			UART_Printf("[NeonRTFS] End of image.\n");
#endif
			return NULL;
		}

		if (h.nameLen <= 0 || h.fileLenComp < 0 || h.fileLenDecomp < 0 ||
		    (size_t)h.nameLen > (size_t)(image_end - p))
		{
			NeonRtFsInitDone = false;
			return NULL;
		}

		const uint8_t *content = p + (size_t)h.nameLen;
		if ((size_t)h.fileLenComp > (size_t)(image_end - content))
		{
			NeonRtFsInitDone = false;
			return NULL;
		}

		size_t copied_name_len = min(sizeof(namebuf) - 1U, (size_t)h.nameLen);
		memcpy(namebuf, p, copied_name_len);
		namebuf[copied_name_len] = '\0'; // 確保以空字元終止

//		info("Found file '%s'. Namelen=%x fileLenComp=%x, compr=%d flags=%d",
//				namebuf, (unsigned int)h.nameLen, (unsigned int)h.fileLenComp, h.compression, h.flags);
#if NEONRTFS_DEBUG==1
		UART_Printf("[NeonRTFS] Found File %s.\n", namebuf);
		UART_Printf("[NeonRTFS] File Length %d Bytes\n", h.fileLenComp);
		UART_Printf("[NeonRTFS] File Content Start at %x\n", p+h.nameLen);
#endif
		if (strcmp(namebuf, fileName)==0)
		{
			//Yay, this is the file we need!
			p = content; //Skip to content.
			r = (NeonRtFsFile *)mem_Malloc(sizeof(NeonRtFsFile)); //Alloc file desc mem
//			UART_Printf("Alloc %p", r);
			if (r==NULL) return NULL;

            memcpy(&r->header, &h, sizeof(NeonRtFsHeader));
			r->decompressor = h.compression;
			r->posComp = p;
			r->posStart = p;
			r->posDecomp = 0;
			if (h.compression==COMPRESS_NONE) {
				r->decompData=NULL;
#ifdef NEONRTFS_HEATSHRINK
				} else if (h.compression==COMPRESS_HEATSHRINK) {
					//File is compressed with Heatshrink.
					if (h.fileLenComp < 1)
					{
						mem_Free(r);
						return NULL;
					}
					char parm;
				heatshrink_decoder *dec;
				//Decoder params are stored in 1st byte.
				readFlashUnaligned(&parm, r->posComp, 1);
				r->posComp++;
#if NEONRTFS_DEBUG==1
				UART_Printf("[NeonRTFS] Heatshrink compressed file; decode parms = %x\n", parm);
#endif
					dec=heatshrink_decoder_alloc(16,
					                             ((uint8_t)parm >> 4) & 0xf,
					                             (uint8_t)parm & 0xf);
					if (dec == NULL)
					{
						mem_Free(r);
						return NULL;
					}
					r->decompData=(char*)dec;
#endif
			} else {
#if NEONRTFS_DEBUG==1
				UART_Printf("[NeonRTFS] Invalid compression: %d\n", h.compression);
#endif
				mem_Free(r);
				return NULL;
			}
#if NEONRTFS_DEBUG==1
			UART_Printf("[NeonRTFS] Open File %s\n", fileName);
#endif
			return r;
		}
		//We don't need this file. Skip name and file
		p = content + (size_t)h.fileLenComp;
		size_t offset = (size_t)(p - s);
		if (offset > SIZE_MAX - 3U)
		{
			NeonRtFsInitDone = false;
			return NULL;
		}
		size_t aligned_offset = (offset + 3U) & ~(size_t)3U;
		if (aligned_offset > neonrtfs_img_len)
		{
			NeonRtFsInitDone = false;
			return NULL;
		}
		p = s + aligned_offset;
	}
}

//Read len bytes from the given file into buff. Returns the actual amount of bytes read.
int NeonRtFsRead(NeonRtFsFile *fh, void *buffer, int len) {
	int32_t flen;
#ifdef NEONRTFS_HEATSHRINK
	int32_t fdlen;
#endif
	if (fh==NULL || buffer == NULL || len <= 0) return 0;
	uint8_t *buff = (uint8_t *)buffer;
	if (fh->header.fileLenComp < 0 || fh->header.fileLenDecomp < 0 ||
	    fh->posStart == NULL || fh->posComp == NULL || fh->posComp < fh->posStart)
	{
		return 0;
	}
#if NEONRTFS_DEBUG==1
        UART_Printf("[NeonRTFS] Read File\n");
#endif
		
        flen = fh->header.fileLenComp;
	//Cache file length.
	//Do stuff depending on the way the file is compressed.
	if (fh->decompressor==COMPRESS_NONE) {
		size_t consumed = (size_t)(fh->posComp - fh->posStart);
		if (consumed >= (size_t)flen)
		{
			return 0;
		}
		size_t toRead = (size_t)flen - consumed;
		if ((size_t)len > toRead) len = (int)toRead;
#if NEONRTFS_DEBUG==1
  		UART_Printf("[NeonRTFS] Reading %d Bytes From %x\n", len, (unsigned int)fh->posComp);
#endif
		readFlashUnaligned(buff, fh->posComp, len);
#if NEONRTFS_DEBUG==1
                UART_Printf("[NeonRTFS] Read Image Data %s\n", buff);
#endif
		fh->posDecomp+=len;
		fh->posComp+=len;
#if NEONRTFS_DEBUG==1
                UART_Printf("[NeonRTFS] Done Reading %d Bytes, Pos=%x\n", len, fh->posComp);
#endif
		return len;
	#ifdef NEONRTFS_HEATSHRINK
		} else if (fh->decompressor==COMPRESS_HEATSHRINK) {
	                fdlen = fh->header.fileLenDecomp;
			int decoded=0;
			char ebuff[16];
			heatshrink_decoder *dec=(heatshrink_decoder *)fh->decompData;
//		UART_Printf("Alloc %p", dec);
			if (dec == NULL || fh->posDecomp < 0 || fh->posDecomp >= fdlen) {
				return 0;
			}

			int32_t output_remaining = fdlen - fh->posDecomp;
			if (len > output_remaining)
			{
				len = (int)output_remaining;
			}

			while(decoded<len) {
				bool made_progress = false;
				size_t consumed = (size_t)(fh->posComp - fh->posStart);

				if (consumed < (size_t)flen)
				{
					size_t input_remaining = (size_t)flen - consumed;
					size_t input_chunk = min(sizeof(ebuff), input_remaining);
					readFlashUnaligned(ebuff, fh->posComp, input_chunk);

					size_t sunk = 0;
					HSD_sink_res sink_result = heatshrink_decoder_sink(
					    dec,
					    (uint8_t *)ebuff,
					    input_chunk,
					    &sunk);
					if (sink_result < 0)
					{
						return decoded;
					}

					fh->posComp += sunk;
					made_progress = sunk > 0;
				}

				size_t polled = 0;
				HSD_poll_res poll_result = heatshrink_decoder_poll(
				    dec,
				    (uint8_t *)buff,
				    (size_t)(len - decoded),
				    &polled);
				if (poll_result < 0)
				{
					return decoded;
				}

				fh->posDecomp += (int32_t)polled;
				buff += polled;
				decoded += (int)polled;
				made_progress = made_progress || polled > 0;

				consumed = (size_t)(fh->posComp - fh->posStart);
				if (consumed >= (size_t)flen)
				{
					HSD_finish_res finish_result = heatshrink_decoder_finish(dec);
					if (finish_result < 0 ||
					    (finish_result == HSDR_FINISH_DONE && polled == 0))
					{
						break;
					}
				}

				if (!made_progress)
				{
					/* Corrupt/truncated streams must not spin forever. */
					break;
				}
			}
			return decoded;
#endif
	}
	return 0;
}

//Close the file.
void NeonRtFsClose(NeonRtFsFile *fh) {
	if (fh==NULL) return;
#if NEONRTFS_DEBUG==1
        UART_Printf("[NeonRTFS] Close File\n");
#endif
#ifdef NEONRTFS_HEATSHRINK
	if (fh->decompressor==COMPRESS_HEATSHRINK) {
		heatshrink_decoder *dec=(heatshrink_decoder *)fh->decompData;
		if (dec != NULL)
		{
			heatshrink_decoder_free(dec);
		}
//		UART_Printf("Freed %p", dec);
	}
#endif
//	UART_Printf("Freed %p", fh);
	mem_Free(fh);
}