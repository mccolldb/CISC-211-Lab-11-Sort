/* ************************************************************************** */
/** Descriptive File Name

  @File Name
    projGlobalDefinitions.h

  @Summary
 defines and constants shared across all files in this project

 */
/* ************************************************************************** */

#ifndef _PROJ_GLOBAL_DEFINITIONS_H    /* Guard against multiple inclusion */
#define _PROJ_GLOBAL_DEFINITIONS_H

/* TODO:  Include other files here if needed. */


/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

    
#define USING_HW 1
    
#define VERBOSE_DEBUG 0

#define MAX_PRINT_LEN 2000

#define MAX_SORT_LEN 10  // excludes trailing 0, so max array size is this + 1
#define MAX_SORT_ARRAY_SIZE ((MAX_SORT_LEN+1))

extern char * nameStrPtr;

// STUDENTS: Uncomment only ONE of the next two lines
#define ONLY_PRINT_SWAP_FAILS ((true))
//#define ONLY_PRINT_SWAP_FAILS ((false))

// STUDENTS: Uncomment only ONE of the next two lines
#define ONLY_PRINT_SORT_FAILS ((true))
// #define ONLY_PRINT_SORT_FAILS ((false))

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _PROJ_GLOBAL_DEFINITIONS_H */

/* *****************************************************************************
 End of File
 */
