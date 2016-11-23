/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  tstpic.h --- Voyager GX SDK 
*  This file contains some picture drawing functions for testing
*  purpose.
* 
*******************************************************************/
#ifndef _TSTPIC_H_
#define _TSTPIC_H_

/*
 * This function clears the screen with black color
 */
void picClearScreen(logicalMode_t *pLogicalMode);

/*
 * This function draws 8 vertical color bars and a while border.
 * Everything is render with SW only.
 */
void picVerticalBars(logicalMode_t *pLogicalMode);

/*
 * This function draws 8 horizontal color bars and a while border.
 * Everything is render with SW only.
 */
void picHorizontalBars(logicalMode_t *pLogicalMode);

/*
 * This function draws horizontal bars with varying intensity from low to high.
 * Everything is render with SW only.
 */
void picHorizontalBarsVaryIntensity(logicalMode_t *pLogicalMode);

/*
 * This function draws vertical bars with varying intensity.
 * Everything is render with SW only.
 */
void picVerticalBarsVaryIntensity(logicalMode_t *pLogicalMode);

/*
 * This function draws horizontal bars with color scaling
 * Everything is render with SW only.
 */
void picHorizontalColorGrid(logicalMode_t *pLogicalMode);

/*
 * This function draws vertical bars with color scaling.
 * Everything is render with SW only.
 */
void picVerticalColorGrid(logicalMode_t *pLogicalMode);

#endif /* _TSTPIC_H_ */
