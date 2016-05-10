/*******************************************************************
* 
*         Copyright (c) 2008 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  INTR.H --- SMI DDK 
*  This file contains the source code for the interrupt mechanism 
* 
*******************************************************************/
#ifndef _INTR_H_
#define _INTR_H_

/*******************************************************************
 * Interrupt implementation
 * 
 * This implementation is used for handling the interrupt.
 *******************************************************************/

typedef void (*PFNINTRHANDLER)(unsigned long);

/*
 * Register an interrupt handler function to an interrupt status
 */ 
short registerHandler(
    void (*handler)(unsigned long status), 
    unsigned long mask
);

/*
 * Un-register a registered interrupt handler
 */
short unregisterHandler(
    void (*handler)(unsigned long status)
);

#endif /* _CHIP_H_ */
