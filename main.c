

#include <common.h>
#include <property.h>
#include <log.h>
#include <odbc.h>
#include <latency.h>

int main(int aArgc, char *aArgv[])
{
    setLogger( const_cast<char*>( "stdout" ) );

    STL_TRY_THROW( aArgc == 2 , RAMP_ERR_INVALID_ARG_COUNT );

    STL_TRY_THROW( setControlFile( aArgv[1] ) == SUCCESS, RAMP_ERR_SET_CONTROLFILE );
    LOGGER( "set controlfile [%s]\n", aArgv[1] );

    STL_TRY_THROW( initProperty() == SUCCESS, RAMP_ERR_INIT_PROPERTY );
    LOGGER( "initialize property, success.\n" );

    STL_TRY( initODBC() == SUCCESS );
    LOGGER( "initialize ODBC, success.\n" );
    
    initTimer();
    LOGGER( "initialize latency data, success.\n" );

    LOGGER( "=== check latency start ===\n" );
    STL_TRY_THROW( checkLatency() == SUCCESS, RAMP_ERR_CHECK_LATENCY );
    LOGGER( "check latency end.\n" );

    return SUCCESS;

    STL_CATCH( RAMP_ERR_INVALID_ARG_COUNT )
    {
        SET_ERROR( "Usage: %s json_file_path\n", aArgv[0]);
    }

    STL_CATCH( RAMP_ERR_SET_CONTROLFILE )
    {
        SET_ERROR( "setControlFile() failed.\n" );
    }

    STL_CATCH( RAMP_ERR_INIT_PROPERTY )
    {
        SET_ERROR( "initProperty() failed.\n" );
    }

    STL_CATCH( RAMP_ERR_CHECK_LATENCY )
    {
        SET_ERROR( "checkLatency() failed.\n" );
    }

    STL_FINISH;
    
    return FAIL;
}
