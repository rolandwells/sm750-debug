#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "smi.h"
#include "smi_712.h"
extern int	entity_priv_index[MAX_ENTITIES];
extern int	free_video_memory;
extern unsigned int	total_video_memory_k;

#ifndef XSERVER_LIBPCIACCESS
unsigned int smi_getmemsize_712(ScrnInfoPtr pScrn, SMIPtr pSmi)
{
	unsigned int	*ptr = NULL, ret;
	char		tmp = 0;

	/*
	 * SM712 only support 2M/4M memory
	 */
	ptr = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_FRAMEBUFFER, pSmi->PciTag, pSmi->PciInfo->memBase[0], 3 * 1024 * 1024);
	if (ptr == NULL) {
		xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "Internal error: could not map " "MMIO registers.\n");
		return (FALSE);
	}
	tmp = 0x12;
	*(ptr + 2 * 1024 * 1024 + 512 * 1024) = tmp;
	if (tmp != (*(ptr + 2 * 1024 * 1024 + 512 * 1024))) {
		ret = 2 * 1024;
	} else {
		ret = 4 * 1024;
	}
	xf86UnMapVidMem(pScrn->scrnIndex, (pointer)pSmi->PciInfo->memBase[0], 3 * 1024 * 1024);
	return ret;
}
#endif

Bool smi_setvideomem_712(int config, ScrnInfoPtr pScrn, SMIPtr pSmi)
{
	int		mem_table[4] = { 1, 2, 4, 0 };
	unsigned int	total_memory = 4 * 1024;

	free_video_memory = total_memory;
//	total_memory = smi_getmemsize_712(pScrn, pSmi);

//	pSmi->videoRAMKBytes = mem_table[(config >> 6)] * 1024;
	pSmi->videoRAMKBytes = total_memory;
	total_video_memory_k = total_memory;
	pSmi->videoRAMBytes = pSmi->videoRAMKBytes * 1024;
	pScrn->videoRam = pSmi->videoRAMKBytes;

#if SMI_DEBUG
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "videoram: %dk\n", pSmi->videoRAMKBytes);
#endif
	return (TRUE);
}

#ifndef XSERVER_LIBPCIACCESS	
Bool smi_mapmemory_712(ScrnInfoPtr pScrn, SMIPtr pSmi)
{
	vgaHWPtr	hwp;
	CARD32		memBase;

	memBase = pSmi->PciInfo->memBase[0] + 0x400000;
	pSmi->MapSize = 0x400000;

	pSmi->MapBase = xf86MapPciMem (pScrn->scrnIndex, VIDMEM_MMIO | VIDMEM_MMIO_32BIT, pSmi->PciTag, memBase, pSmi->MapSize);
	if (pSmi->MapBase == NULL)
	{
		xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "Internal error: could not map "
			"MMIO registers.\n");
		return (FALSE);
	}

	pSmi->DPRBase = pSmi->MapBase + 0x008000;
	pSmi->VPRBase = pSmi->MapBase + 0x00C000;
	pSmi->CPRBase = pSmi->MapBase + 0x00E000;
	pSmi->IOBase = pSmi->MapBase + 0x300000;
	pSmi->DataPortBase = pSmi->MapBase /*+ 0x100000 */ ;
	pSmi->DataPortSize = 0x8000 /*0x200000 */ ;

	pScrn->memPhysBase = pSmi->PciInfo->memBase[0];
	pSmi->fbMapOffset = 0x0;

	if (pSmi->videoRAMBytes) {
		pSmi->FBBase = xf86MapPciMem(pScrn->scrnIndex,
			VIDMEM_FRAMEBUFFER, pSmi->PciTag,
			pScrn->memPhysBase + pSmi->fbMapOffset,
			pSmi->videoRAMBytes);
		if (pSmi->FBBase == NULL) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
					"Internal error: could not "
					"map framebuffer.\n");
			return (FALSE);
		}
	}
	pSmi->FBOffset = 0;
	pScrn->fbOffset = pSmi->FBOffset + pSmi->fbMapOffset;
	SMI_EnableMmio(pScrn);

	pSmi->FBCursorOffset = pSmi->videoRAMBytes - 1024;
	pSmi->FBReserved = pSmi->videoRAMBytes - 2048;
	if (VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x30) & 0x01) {
		CARD32	fifoOffset = 0;
		fifoOffset |= VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x46) << 3;
		fifoOffset |= VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x47) << 11;
		fifoOffset |= (VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x49) & 0x1C) << 17;
		pSmi->FBReserved = fifoOffset;
	}
#if LCD_SIZE_DETECT
	pSmi->lcd = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31) & 0x01;
	if (VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x30) & 0x01) {
		pSmi->lcd <<= 1;
	}
	switch (VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x30) & 0x0C) {
		case 0x00:
			pSmi->lcdWidth = 640;
			pSmi->lcdHeight = 480;
		break;

		case 0x04:
			pSmi->lcdWidth = 800;
			pSmi->lcdHeight = 600;
		break;

		case 0x08:
			if (VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x74) & 0x02) {
				pSmi->lcdWidth = 1024;
				pSmi->lcdHeight = 600;
			} else {
				pSmi->lcdWidth = 1024;
				pSmi->lcdHeight = 768;
			}
		break;

		case 0x0C:
			pSmi->lcdWidth = 1280;
			pSmi->lcdHeight = 1024;
		break;
	}
#else
xf86DrvMsg("", X_INFO, "Belcon:smi_712.c, lcdWidth %d, virtualX %d\n", pSmi->lcdWidth, pScrn->virtualX);
	if (!pSmi->lcdWidth)
		pSmi->lcdWidth = pScrn->virtualX;
	if (!pSmi->lcdHeight)
		pSmi->lcdHeight = pScrn->virtualY;
#endif

	return (TRUE);
}
#endif
