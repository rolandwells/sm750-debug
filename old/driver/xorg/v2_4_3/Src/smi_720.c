#include "smi.h"
#include "smi_720.h"
#ifndef XSERVER_LIBPCIACCESS




Bool smi_setvideomem_720(int config, ScrnInfoPtr pScrn, SMIPtr pSmi)
{
	int	mem_table[4] = { 16, 2, 4, 8 };
	pSmi->videoRAMKBytes = mem_table[(config >> 6)] * 1024;
	pSmi->videoRAMBytes = pSmi->videoRAMKBytes * 1024;
	pScrn->videoRam = pSmi->videoRAMKBytes;

	return (TRUE);
}

Bool smi_mapmemory_720(ScrnInfoPtr pScrn, SMIPtr pSmi)
{
	vgaHWPtr	hwp;
	CARD32		memBase;

	memBase = pSmi->PciInfo->memBase[0];
	pSmi->MapSize = 0x200000;

	pSmi->MapBase = xf86MapPciMem (pScrn->scrnIndex, VIDMEM_MMIO | VIDMEM_MMIO_32BIT, pSmi->PciTag, memBase, pSmi->MapSize);
	if (pSmi->MapBase == NULL) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				"Internal error: could not map "
				"MMIO registers.\n");
		return (FALSE);
	}

	pSmi->DPRBase = pSmi->MapBase + 0x000000;
	pSmi->VPRBase = pSmi->MapBase + 0x000800;
	pSmi->CPRBase = pSmi->MapBase + 0x001000;
	pSmi->IOBase = pSmi->MapBase + 0x0C0000;
	pSmi->DataPortBase = pSmi->MapBase + 0x100000;
	pSmi->DataPortSize = 0x100000;

	pScrn->memPhysBase = pSmi->PciInfo->memBase[0];
	pSmi->fbMapOffset = 0x200000;

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
	if (!pSmi->lcdWidth)
		pSmi->lcdWidth = pScrn->virtualX;
	if (!pSmi->lcdHeight)
		pSmi->lcdHeight = pScrn->virtualY;
#endif
	return (TRUE);
}


#endif
