#include "smi.h"
#include "smi_730.h"
#ifndef XSERVER_LIBPCIACCESS	

Bool smi_setvideomem_730(int config, ScrnInfoPtr pScrn, SMIPtr pSmi)
{
	pSmi->videoRAMKBytes = 16 * 1024;
	pSmi->videoRAMBytes = pSmi->videoRAMKBytes * 1024;
	pScrn->videoRam = pSmi->videoRAMKBytes;

	return (TRUE);
}

Bool smi_mapmemory_730(ScrnInfoPtr pScrn, SMIPtr pSmi)
{
	vgaHWPtr	hwp;
	CARD32		memBase;

	memBase = pSmi->PciInfo->memBase[1];
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
	pSmi->FPRBase = pSmi->MapBase + 0x005800;
	pSmi->IOBase = pSmi->MapBase + 0x0C0000;
	pSmi->DataPortBase = pSmi->MapBase + 0x100000;
	pSmi->DataPortSize = 0x100000;

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

	if (pSmi->pInt10 != NULL) {
		pSmi->pInt10->num = 0x10;
		pSmi->pInt10->ax = 0x5F00;
		pSmi->pInt10->bx = 0;
		pSmi->pInt10->cx = 0;
		pSmi->pInt10->dx = 0;
		xf86ExecX86int10 (pSmi->pInt10);
		if (pSmi->pInt10->ax == 0x005F) {
			switch (pSmi->pInt10->cx & 0x0F) {
				case PANEL_640x480:
					pSmi->lcdWidth = 640;
					pSmi->lcdHeight = 480;
				break;
				case PANEL_800x600:
					pSmi->lcdWidth = 800;
					pSmi->lcdHeight = 600;
				break;
				case PANEL_1024x768:
					pSmi->lcdWidth = 1024;
					pSmi->lcdHeight = 768;
				break;
				case PANEL_1280x1024:
					pSmi->lcdWidth = 1280;
					pSmi->lcdHeight = 1024;
				break;
				case PANEL_1600x1200:
					pSmi->lcdWidth = 1600;
					pSmi->lcdHeight = 1200;
				break;
				case PANEL_1400x1050:
					pSmi->lcdWidth = 1400;
					pSmi->lcdHeight = 1050;
				break;
			}
			xf86DrvMsg (pScrn->scrnIndex, X_INFO,
					"Detected panel size via BIOS: "
					"%d x %d\n",
					pSmi->lcdWidth, pSmi->lcdHeight);
		} else {
			xf86DrvMsg (pScrn->scrnIndex, X_INFO,
					"BIOS error during 730 panel "
					"detection!\n");
			pSmi->lcdWidth = pScrn->virtualX;
			pSmi->lcdHeight = pScrn->virtualY;
		}
	} else {
		/*
		 * int10 support isn't setup on the second call to
		 * this function,so if this is the second call,
		 * don't do detection again
		 */
		if (pSmi->lcd == 0) {
			/*
			 * If we get here, int10 support is not loaded
			 * or not working
			 */
			xf86DrvMsg(pScrn->scrnIndex, X_INFO,
				"No BIOS support for 730 panel detection!\n");
			pSmi->lcdWidth = pScrn->virtualX;
			pSmi->lcdHeight = pScrn->virtualY;
		}
	}
	pSmi->lcd = 1;

	return (TRUE);
}
#endif
