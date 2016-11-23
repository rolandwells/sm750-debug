#include "smi.h"
#include "smi_chips.h"
struct pci_dev {
	struct pci_dev 	*next;
	unsigned short domain;
	unsigned char bus, dev, func;

	int known_fields;
	unsigned short vendor_id, device_id;
	int irq;
	unsigned int base_addr[6];
	unsigned int size[6];
	unsigned int rom_base_addr;
	unsigned int rom_size;

//	struct pci_access *access;
//	struct pci_methods *methods;
	unsigned char *cache;
	int cache_len;
	int hdrtype;
	void *aux;
};
extern int	entity_priv_index[MAX_ENTITIES];
Bool smi_setdepbpp(ScrnInfoPtr pScrn)
{
#ifdef XORG_VERSION_CURRENT
	#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
	if (!xf86SetDepthBpp (pScrn, 0, 0, 0, Support32bppFb)) {
	#else
	if (!xf86SetDepthBpp (pScrn, 0, 0, 0, Support24bppFb)) {
	#endif
#else
	if (!xf86SetDepthBpp (pScrn, 0, 0, 0, Support24bppFb)) {
#endif
		return (FALSE);
	}

	/* Check that the returned depth is one we support */
	switch (pScrn->depth) {
		case 8:
		case 16:
		case 24:
		break;

		default:
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				"Given depth (%d) is not supported "
				"by this driver\n", pScrn->depth);
			return (FALSE);
		break;
	}
	return (TRUE);
}

Bool smi_setvideomem(int config, ScrnInfoPtr pScrn, SMIPtr pSmi)
{
	int		mem_table[4] = { 1, 2, 4, 0 };

	pSmi->videoRAMKBytes = mem_table[(config >> 6)] * 1024;
	pSmi->videoRAMBytes = pSmi->videoRAMKBytes * 1024;
	pScrn->videoRam = pSmi->videoRAMKBytes;

#if SMI_DEBUG
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "videoram: %dk\n", pSmi->videoRAMKBytes);
#endif
	return (TRUE);
}

void smi_setclk(ScrnInfoPtr pScrn, int clock0, int clock1, int clock2, int clock3)
{
	if (pScrn->clock[0] <= 0)
		pScrn->clock[0] = clock0;
	if (pScrn->clock[1] <= 0)
		pScrn->clock[1] = clock1;
	if (pScrn->clock[2] <= 0)
		pScrn->clock[2] = clock2;
	if (pScrn->clock[3] <= 0)
		pScrn->clock[3] = clock3;

	return;
}

Bool smi_mapmemory(ScrnInfoPtr pScrn, SMIPtr pSmi)
{
	vgaHWPtr	hwp;
	CARD32		memBase;
	
#ifndef XSERVER_LIBPCIACCESS
	memBase = pSmi->PciInfo->memBase[0] + 0x400000;
#else
	memBase = PCI_REGION_BASE(pSmi->PciInfo,0,REGION_MEM) + 0x400000;//caesar modified
#endif

	pSmi->MapSize = 0x10000;

	pSmi->MapBase = xf86MapPciMem (pScrn->scrnIndex, VIDMEM_MMIO | VIDMEM_MMIO_32BIT, pSmi->PciTag, memBase, pSmi->MapSize);
	if (pSmi->MapBase == NULL) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				"Internal error: could not map "
				"MMIO registers.\n");
		return (FALSE);
	}

	pSmi->DPRBase = pSmi->MapBase + 0x8000;
	pSmi->VPRBase = pSmi->MapBase + 0xC000;
	pSmi->CPRBase = pSmi->MapBase + 0xE000;
	pSmi->IOBase = NULL;
	pSmi->DataPortBase = pSmi->MapBase;
	pSmi->DataPortSize = 0x8000;

#ifndef XSERVER_LIBPCIACCESS
	pScrn->memPhysBase = pSmi->PciInfo->memBase[0];
#else
	pScrn->memPhysBase = PCI_REGION_BASE(pSmi->PciInfo,0,REGION_MEM);//caesar modified
#endif

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
	if (!pSmi->lcdWidth)
		pSmi->lcdWidth = pScrn->virtualX;
	if (!pSmi->lcdHeight)
		pSmi->lcdHeight = pScrn->virtualY;
#endif
}

void read_cmd_reg(int offset)
{
	FILE	*fp;
	int	fd;
	char	buf[512];
	struct pci_dev	*d;

#if 0	

	d = calloc(1, sizeof(struct pci_dev));
	fp = fopen("/proc/bus/pci/devices", "r");
	if (!fp) {
		xf86DrvMsg("", X_INFO, "Failed to open proc file\n");
		return;
	}
	while (fgets(buf, sizeof(buf)-1, fp)) {
		unsigned int	dfn, vend, cnt, known;
		cnt = sscanf(buf,
			"%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x",
			&dfn, &vend, &d->irq,
			&d->base_addr[0],
			&d->base_addr[1],
			&d->base_addr[2],
			&d->base_addr[3],
			&d->base_addr[4],
			&d->base_addr[5],
			&d->rom_base_addr,
			&d->size[0],
			&d->size[1],
			&d->size[2],
			&d->size[3],
			&d->size[4],
			&d->size[5],
			&d->rom_size);
		if (cnt != 9 && cnt != 10 && cnt != 17) {
			printf("proc: parse error (read only %d items)", cnt);
		}
		d->bus = dfn >> 8U;
		d->dev = (dfn >> 3) & 0x1f;
		d->func = (dfn) & 0x07;
		d->vendor_id = vend >> 16U;
		d->device_id = vend & 0xffff;
		known = 1;
		if (d->vendor_id != 0x126f)
			continue;

		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, sizeof(buf), "/proc/bus/pci/%02x/%02x.%d",
			d->bus, d->dev, d->func);
		fd = open(buf, O_RDONLY);
		if (fd < 0) {
			perror("open");
			break;
		}
		read(fd, &buf, 256);
	//	printf("%d:%d:%d : value of offset 0x%x is 0x%x\n", d->bus, d->dev, d->func, offset, buf[offset]);
		xf86DrvMsg("", X_INFO, "%d:%d:%d : value of offset 0x%x is 0x%x\n", d->bus, d->dev, d->func, offset, buf[offset]);
		close(fd);
	}

	fclose(fp);
	free(d);
#endif
}
