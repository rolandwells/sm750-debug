/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  VERSION.H --- SMI DDK 
*  This file contains the source code for the mode table.
* 
*******************************************************************/
#ifndef _VERSION_H_
#define _VERSION_H_

#define LIBRARY_VERSION 0x00000006
#define MAJOR_VERSION(v) (v >> 16)
#define MINOR_VERSION(v) (v & 0x0000ffff)
#if XORG_VERSION_CURRENT < 10706000
uint32_t getLibVersion(void)
{
	return(LIBRARY_VERSION);
}
#endif
#endif /* _VERSION_H_ */

