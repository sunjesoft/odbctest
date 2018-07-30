
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef _COMMON_H_
#define _COMMON_H_

#define SUCCESS (0)
#define FAIL    (-1)

#define TRUE    (0)
#define FALSE   (-1)

#define STL_THROW( aLabel ) goto aLabel;

#define STL_TRY_THROW( aExpression, aLabel )    \
    do                                          \
    {                                           \
        if( !(aExpression) )                    \
        {                                       \
            goto aLabel;                        \
        }                                       \
    } while( 0 )

#define STL_TRY( aExpression )                  \
    do                                          \
    {                                           \
        if( !(aExpression) )                    \
        {                                       \
            goto STL_FINISH_LABEL;              \
        }                                       \
    } while( 0 )


#define STL_CATCH( aLabel )                     \
    goto STL_FINISH_LABEL;                      \
    aLabel:

#define STL_RAMP( aLabel ) aLabel:


#define STL_FINISH                              \
    goto STL_FINISH_LABEL;                      \
    STL_FINISH_LABEL:

#endif
