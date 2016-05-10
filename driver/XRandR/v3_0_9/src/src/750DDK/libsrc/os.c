/*******************************************************************
* 
*         Copyright (c) 2008 by Silicon Motion, Inc. (SMI)
* 
* OS.C --- VGX family DDK
*
* OS or platform dependent files are conditionally compiled here.
*
*******************************************************************/
#include "os.h"

/*
 * Use WATCOM DOS Extender compiler to implement the functions in OS.H
 */
#ifdef WDOSE
#include "WATCOM.C"
#endif


/* 
 * Use Linux compiler to implement the functions in OS.H
 */
#ifdef LINUX
#include "linux.c"
#endif
