/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  DISPINT.H --- Voyager GX SDK 
* 
*******************************************************************/

#ifndef _DISPINT_H_
#define _DISPINT_H_

#include "display.h"
#include "intr.h"

/* 
 * Register Vertical Sync Interrupt function 
 */
short hookVSyncInterrupt(
    disp_control_t dispControl,
    void (*handler)(void)
);

/* 
 * Unregister a registered VSync interrupt handler 
 */
short unhookVSyncInterrupt(
    disp_control_t dispControl,
    void (*handler)(void)
);

#endif  /* _DISPINT_H_ */
