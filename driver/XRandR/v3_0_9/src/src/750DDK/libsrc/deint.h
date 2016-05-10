/*
 * Voyager GX SDK
 *
 * deint.h
 */
#ifndef _DE_INT_H_
#define _DE_INT_H_

/*
 * This function register 2D Interrupt handler.
 */
int32_t hook2DInterrupt(
    void (*handler)(void)
);

/*
 * This function un-register 2D Interrupt handler.
 */
int32_t unhook2DInterrupt(
    void (*handler)(void)
);

/*
 * This function register CSC Interrupt handler.
 */
int32_t hookCSCInterrupt(
    void (*handler)(void)
);

/*
 * This function un-register CSC Interrupt handler.
 */
int32_t unhookCSCInterrupt(
    void (*handler)(void)
);

#endif /* _DE_INT_H_ */
