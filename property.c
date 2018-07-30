

#include <property.h>

Property gProperty;
char gControlFile[ MAX_CONTROL_FILE_LEN ];
json_object *gJSonObject;



int setControlFile( const char* aControlFile )
{
    STL_TRY_THROW( aControlFile != NULL, RAMP_ERR_PARAM_NULL );

    STL_TRY_THROW( strlen( aControlFile ) > 0, RAMP_ERR_PARAM_TOO_LONG );

    strcpy( gControlFile , aControlFile );

    return SUCCESS;

    STL_CATCH( RAMP_ERR_PARAM_TOO_LONG )
    {
        SET_ERROR( "input control path is too long." );
    }

    STL_CATCH( RAMP_ERR_PARAM_NULL )
    {
        SET_ERROR( "input control path is null." );
    }

    STL_FINISH;

    return FAIL;
}

char* readControlFile( const char* aControlFile )
{
    struct stat sJSonFileStat;
    char* sJSonData = NULL;
    FILE* sFd       = NULL;
    int   sRet      = 0;

    STL_TRY_THROW( aControlFile != NULL,
                   RAMP_ERR_PARAM_NULL );

    STL_TRY_THROW( ( stat( aControlFile , &sJSonFileStat ) == 0 ),
                   RAMP_ERR_STAT );
        
    STL_TRY_THROW( (sJSonData = calloc( 1, sJSonFileStat.st_size + 1 )) != NULL,
                   RAMP_ERR_MEMORY_ALLOC_FAILED );

    STL_TRY_THROW( (sFd = fopen( aControlFile , "r" )) != NULL,
                   RAMP_ERR_OPEN );

    sRet = fread( (void*)sJSonData , 1, sJSonFileStat.st_size, sFd );
    STL_TRY_THROW( sRet = sJSonFileStat.st_size, RAMP_ERR_READ );

    return sJSonData;

    STL_CATCH( RAMP_ERR_PARAM_NULL )
    {
        SET_ERROR( "control file path is null." );
    }

    STL_CATCH( RAMP_ERR_STAT )
    {
        SET_ERROR( "stat() failed: %s", strerror( errno ) );
    }

    STL_CATCH( RAMP_ERR_MEMORY_ALLOC_FAILED )
    {
        SET_ERROR( "calloc() failed: %s", strerror( errno ) );
    }

    STL_CATCH( RAMP_ERR_OPEN )
    {
        SET_ERROR( "fopen() failed: %s", strerror( errno ) );
    }
        
    STL_CATCH( RAMP_ERR_READ )
    {
        SET_ERROR( "fread() failed: %s", strerror( errno ) );
    }

    STL_FINISH;

    if( sJSonData != NULL )
    {
        free( sJSonData );
    }

    return NULL;
}

void printProperty()
{
    LOGGER( "=== Property ===" );
    LOGGER( "" );
    LOGGER( "DATABASE        : %s", gProperty.mDatabase );
    LOGGER( "DSN             : %s", gProperty.mDSN );
    LOGGER( "UID             : %s", gProperty.mUID );
    LOGGER( "PWD             : %s", gProperty.mPWD );
    LOGGER( "ARRAY           : %d", gProperty.mArray );
    LOGGER( "REPEAT          : %d", gProperty.mRepeat );
    LOGGER( "COMMIT_INTERVAL : %d", gProperty.mCommitInterval );

    if( gProperty.mAutoCommit == TRUE )
    {
        LOGGER( "AUTOCOMMIT      : ON" );
    }
    else
    {
        LOGGER( "AUTOCOMMIT      : OFF" );
    }

    if( gProperty.mExecuteType == DIRECT_EXECUTE )
    {
        LOGGER( "EXECUTE_TYPE    : DIRECT_EXECUTE" );
    }
    else
    {
        LOGGER( "EXECUTE_TYPE    : PREPARE_EXECUTE" );
    }

    LOGGER( "SQL             : %s", gProperty.mQuery );
    
}

void _initProperty()
{
    memset( &gProperty , 0x00 , sizeof( Property ) );

    strcpy( gProperty.mDatabase, "SUNDB" );
    strcpy( gProperty.mDSN     , "SUNDB" );
    strcpy( gProperty.mUID     , "test" );
    strcpy( gProperty.mPWD     , "test" );
    gProperty.mArray          = 1;
    gProperty.mRepeat         = 1;
    gProperty.mAutoCommit     = FALSE;
    gProperty.mExecuteType    = PREPARE_EXECUTE;
    gProperty.mCommitInterval = 1;
    gProperty.mQueryType      = 0;
}

int doJSonParsing( json_object * aJSonObject )
{
    json_object* sJSonObject;
    json_type  sType;

    json_object_object_foreach( aJSonObject , sKey , sVal )
    {
        
        if( strlen( sKey ) > MAX_KEY_LEN )
        {
            SET_ERROR( "key [%s] is too long.", sKey );
            STL_THROW( STL_FINISH_LABEL );
        }

        if( strcasecmp( sKey, "SQL" ) != 0 &&
            strlen( json_object_get_string( sVal ) ) > MAX_VALUE_LEN )
        {
            SET_ERROR( "[%s]'s value [%s] is too long.", sKey , json_object_get_string( sVal ) );
            STL_THROW( STL_FINISH_LABEL );
        }
        
        sType = json_object_get_type( sVal );
        switch( sType )
        {
            case json_type_boolean:
            case json_type_double: 
            case json_type_int: 
            case json_type_string:
                break;

            case json_type_object:
                strcpy( gProperty.mDatabase , sKey );

                sJSonObject = json_object_object_get( aJSonObject, sKey );
                doJSonParsing( sJSonObject );
                continue;
                break;
                
            case json_type_array:
                SET_ERROR( "array type not support." );
                STL_THROW( STL_FINISH_LABEL );
                break;

            default:
                SET_ERROR( "invalid json_type. %d", (int)sType );
                STL_THROW( STL_FINISH_LABEL );
                break;
        }    
                
        if( strcasecmp( sKey, "DSN" ) == 0 )
        {
            strcpy( gProperty.mDSN, json_object_get_string( sVal ) );
        }
        else if( strcasecmp( sKey, "UID" ) == 0 )
        {
            strcpy( gProperty.mUID, json_object_get_string( sVal ) );
        }
        else if( strcasecmp( sKey, "PWD" ) == 0 )
        {
            strcpy( gProperty.mPWD, json_object_get_string( sVal ) );
        }
        else if( strcasecmp( sKey, "ARRAY" ) == 0 )
        {
            gProperty.mArray = json_object_get_int( sVal );
        }
        else if( strcasecmp( sKey, "COMMIT_INTERVAL" ) == 0 )
        {
            gProperty.mCommitInterval = json_object_get_int( sVal );
        }
        else if( strcasecmp( sKey, "REPEAT" ) == 0 )
        {
            gProperty.mRepeat = json_object_get_int( sVal );
        }
        else if( strcasecmp( sKey, "AUTOCOMMIT" ) == 0 )
        {
            if( strcasecmp( json_object_get_string( sVal ), "ON" ) == 0 )
            {
                gProperty.mAutoCommit = TRUE;
            }
            else if( strcasecmp( json_object_get_string( sVal ), "OFF" ) == 0 )
            {
                gProperty.mAutoCommit = FALSE;
            }
            else
            {
                SET_ERROR( "Autocommit value [%s] is invalid.", json_object_get_string( sVal ) );
                STL_THROW( STL_FINISH_LABEL );
            }
        }
        else if( strcasecmp( sKey, "IS_DIRECT_EXECUTE" ) == 0 )
        {
            if( strcasecmp( json_object_get_string( sVal ), "YES" ) == 0 )
            {
                gProperty.mExecuteType = DIRECT_EXECUTE;
            }
            else if( strcasecmp( json_object_get_string( sVal ), "NO" ) == 0 )
            {
                gProperty.mExecuteType = PREPARE_EXECUTE;
            }
            else
            {
                SET_ERROR( "is_direct_execute value [%s] is invalid.", json_object_get_string( sVal ) );
                STL_THROW( STL_FINISH_LABEL );
            }
        }
        else if( strcasecmp( sKey, "SQL" ) == 0 )
        {
            if( strlen( json_object_get_string( sVal ) ) > MAX_QUERY_LEN )
            {
                SET_ERROR( "user sql is too long, max value is 8192 byte." );
                STL_THROW( STL_FINISH_LABEL );
            }

            strcpy( gProperty.mQuery , json_object_get_string( sVal ) );
        }
        else if( strcasecmp( sKey, "OUTFILE" ) == 0 )
        {
            if( strlen( json_object_get_string( sVal ) ) > MAX_QUERY_LEN )
            {
                SET_ERROR( "user sql is too long, max value is 8192 byte." );
                STL_THROW( STL_FINISH_LABEL );
            }

            strcpy( gProperty.mOutfile , json_object_get_string( sVal ) );
        }

        else
        {
            SET_ERROR( "key [%s] is invalid option name.", sKey );
            STL_THROW( STL_FINISH_LABEL );
        }
        
    }

    return SUCCESS;

    STL_FINISH;

    return FAIL;
}

int initProperty()
{
    json_object* sJSonObject;
    char* sJSonData = NULL;
       
    _initProperty();

    STL_TRY_THROW( (sJSonData = readControlFile( gControlFile )) != NULL,
                   RAMP_ERR_READ_CONTROLFILE );
   
    STL_TRY_THROW( (sJSonObject = json_tokener_parse( sJSonData )) != NULL,
                   RAMP_ERR_JSON_TOKENER );

    doJSonParsing( sJSonObject );

    printProperty();

    json_object_put( sJSonObject );

    if( sJSonData != NULL)
    {
        free( sJSonData );
    }

    return SUCCESS;

    STL_CATCH( RAMP_ERR_READ_CONTROLFILE )
    {
        SET_ERROR( "readControlFile() failed." );
    }

    STL_CATCH( RAMP_ERR_JSON_TOKENER )
    {
        SET_ERROR( "json_tokener_parse() error" );
    }
  
    STL_FINISH;

    if( sJSonData != NULL )
    {
        free( sJSonData );
    }

    return FAIL;
}
