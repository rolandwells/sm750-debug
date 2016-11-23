/*******************************************************************
* 
*         Copyright (c) 2009 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  dviint.h --- SMI DDK 
*  This file contains the source code for the DVI controller chip
*  interrupt
* 
*******************************************************************/
#ifndef _DVI_INT_H_
#define _DVI_INT_H_

#include "dvi.h"

/*  
 *  hookDVIHotPlugInterrupt
 *      Register DVI Hot Plug Interrupt function.
 */
long hookDVIHotPlugInterrupt(
    void (*handler)(void)
);

/*  
 *  unhookDVIHotPlugInterrupt
 *      unregister DVI Hot Plug Interrupt function.
 */
long unhookDVIHotPlugInterrupt(
    void (*handler)(void)
);

#endif  /* _DVI_CTRL_H_ */
