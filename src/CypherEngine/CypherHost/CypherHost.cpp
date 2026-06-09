/*======================================================================
   File: host_main.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-04-19 01:23:58
   Last Modified by: ksiric
   Last Modified: 2026-05-10 23:44:08
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
																	   */
#include "CypherEngine/CypherCommand/CypherCommand.h"
#include "CypherEngine/CypherCVar/CypherCVar.h"
#include "CypherEngine/CypherCommon/CypherCommon_Print.h"
#include "CypherEngine/CypherHost/CypherHost.h"
#include "CypherEngine/CypherConfig/CypherConfig.h"
#include "CypherEngine/CypherFileSystem/CypherFileSystem.h"
#include "CypherEngine/CypherLog/CypherLog.h"
#include "CypherEngine/CypherRender/CypherRender.h"
#include "CypherEngine/CypherSystem/CypherSystem_Platform.h"

namespace rc = cypher::engine::common;

namespace {

/*
================
Host Builtin Commands
================
*/
void CypherHost_CmdEcho( void *extra_data, rc::u32 argc, char **argv ) {
    ( void )extra_data;
    for ( rc::u32 i = 1u; i < argc; ++i ) {
        rc::CypherCommon_Printf( "%s%s", argv[i], ( i + 1u < argc ) ? " " : "\n" );
    }
    if ( argc <= 1u ) {
        rc::CypherCommon_Printf( "\n" );
    }
}

void CypherHost_CmdVersion( void *extra_data, rc::u32 argc, char **argv ) {
    ( void )extra_data;
    ( void )argc;
    ( void )argv;

    const rc::version_t &engine_version = rc::COM_ENGINE_INFO.version;
    const rc::version_t &game_version = rc::COM_GAME_INFO.version;

    rc::CypherCommon_Printf(
        "%s %u.%u.%u.%u | %s %u.%u.%u.%u\n",
        rc::COM_ENGINE_INFO.name,
        engine_version.major,
        engine_version.minor,
        engine_version.patch,
        engine_version.build,
        rc::COM_GAME_INFO.name,
        game_version.major,
        game_version.minor,
        game_version.patch,
        game_version.build );
}

void CypherHost_CmdQuit( void *extra_data, rc::u32 argc, char **argv ) {
    ( void )argc;
    ( void )argv;

    cypher::engine::host::state_t *host_state = static_cast<cypher::engine::host::state_t *>( extra_data );

    if ( host_state == nullptr ) {
        return ;
    }

    CypherHost_RequestShutdown( *host_state );
}

}       // namespace

namespace cypher::engine::host {

/*
================
CypherHost_PrepareStateForInit
================
*/
void CypherHost_PrepareStateForInit( state_t &host_state ) {
    host_state.stage = stage_t::INITIALIZING;
    host_state.running = false;
    host_state.has_focus = true;
    host_state.frame = {};
}

/*
================
CypherHost_InitCoreEngineSystems

Brings up low-level systems in dependency order.
================
*/
error_code_t CypherHost_InitCoreEngineSystems( state_t &host_state ) {
    sys::init_info_t sys_info {
        .argc = host_state.config.argc,
        .argv = host_state.config.argv,
        .app_name = common::COM_GAME_INFO.internal_name,
        .organization_name = common::COM_GAME_INFO.organization_name
    };

    const auto sys_result = sys::CypherSystem_Init( sys_info );
    if ( sys_result != sys::error_code_t::OK ) {
        common::CypherCommon_Errorf( CypherSystem_ErrorCode( sys_result ) , "CypherHost_Init: CypherSystem_Init failed: %s", sys::CypherSystem_ErrorDesc( sys_result ) );

        host_state.running = false;
        host_state.stage = stage_t::SHUTDOWN;
        return error_code_t::ERR_INITIALIZING;
    }

    const auto log_result = log::CypherLog_Init();
    if ( log_result != log::error_code_t::OK ) {
        common::CypherCommon_Errorf( log::CypherLog_ErrorCode( log_result ), "CypherHost_Init: CypherLog_Init failed: %s", log::CypherLog_ErrorDesc( log_result ) );

        sys::CypherSystem_Shutdown();

        host_state.running = false;
        host_state.stage = stage_t::SHUTDOWN;
        return error_code_t::ERR_INITIALIZING;
    }

    CYPHER_LOG_INFO( log::channel_t::HOST, "%s startup begin.", common::COM_ENGINE_INFO.name );
    CYPHER_LOG_INFO( log::channel_t::SYSTEM, "system initialized: platform=%s, compiler=%s.", sys::CypherSystem_PlatformName( sys::CypherSystem_PlatformType() ), sys::CypherSystem_CompilerName( sys::CypherSystem_CompilerType() ) );
    CYPHER_LOG_INFO( log::channel_t::SYSTEM, "paths: base='%s', user='%s', executable='%s'.", sys::CypherSystem_Paths().base_path, sys::CypherSystem_Paths().user_path, sys::CypherSystem_Paths().executable_path );

    const auto fs_result = fs::CypherFileSystem_Init();
    if( fs_result != fs::error_code_t::OK ) {
        CYPHER_LOG_ERROR( log::channel_t::FS, "filesystem initialization failed: %s.", fs::CypherFileSystem_ErrorDesc( fs_result ) );
        common::CypherCommon_Errorf( CypherFileSystem_ErrorCode( fs_result ), "CypherHost_Init: CypherFileSystem_Init failed: %s", fs::CypherFileSystem_ErrorDesc( fs_result ) );
        log::CypherLog_Shutdown();
        sys::CypherSystem_Shutdown();

        host_state.running = false;
        host_state.stage = stage_t::SHUTDOWN;
        return error_code_t::ERR_INITIALIZING;
    }
    const auto cmd_result = cmd::CypherCommand_Init();
    if ( cmd_result != cmd::error_code_t::OK )
    {
        CYPHER_LOG_ERROR( log::channel_t::CMD, "command system initialization failed: %s.", CypherCommand_ErrorDesc( cmd_result ) );
        common::CypherCommon_Errorf( CypherCommand_ErrorCode( cmd_result ), "CypherHost_Init: CypherCommand_Init failed: %s", CypherCommand_ErrorDesc( cmd_result ) );

        fs::CypherFileSystem_Shutdown();
        log::CypherLog_Shutdown();
        sys::CypherSystem_Shutdown();

        host_state.running = false;
        host_state.stage = stage_t::SHUTDOWN;
        return error_code_t::ERR_INITIALIZING;
    }

    const auto cvar_result = cvar::CypherCVar_Init();

    if ( cvar_result != cvar::error_code_t::OK )
    {
        CYPHER_LOG_ERROR( log::channel_t::CVAR, "cvar system initialization failed: %s.", cvar::CypherCVar_ErrorDesc( cvar_result ) );
        common::CypherCommon_Errorf( CypherCVar_ErrorCode( cvar_result ), "CypherHost_Init: CypherCVar_Init failed: %s", cvar::CypherCVar_ErrorDesc( cvar_result ) );

        cmd::CypherCommand_Shutdown();
        fs::CypherFileSystem_Shutdown();
        log::CypherLog_Shutdown();
        sys::CypherSystem_Shutdown();

        host_state.running = false;
        host_state.stage = stage_t::SHUTDOWN;
        return error_code_t::ERR_INITIALIZING;
    }

    const auto cfg_result = cfg::CypherConfig_Init();

    if ( cfg_result != cfg::error_code_t::OK )
    {
        CYPHER_LOG_ERROR( log::channel_t::CFG, "config system initialization failed: %s.", cfg::CypherConfig_ErrorDesc( cfg_result ) );
        common::CypherCommon_Errorf( CypherConfig_ErrorCode( cfg_result ), "CypherHost_Init: CypherConfig_Init failed: %s", cfg::CypherConfig_ErrorDesc( cfg_result ) );

        cvar::CypherCVar_Shutdown();
        cmd::CypherCommand_Shutdown();
        fs::CypherFileSystem_Shutdown();
        log::CypherLog_Shutdown();
        sys::CypherSystem_Shutdown();

        host_state.running = false;
        host_state.stage = stage_t::SHUTDOWN;
        return error_code_t::ERR_INITIALIZING;
    }

    CYPHER_LOG_INFO( log::channel_t::HOST, "core engine systems initialized." );
    return error_code_t::OK;
}

/*
================
CypherHost_MountFileSystem
================
*/
error_code_t CypherHost_MountFileSystem( void ) {
    const sys::paths_t &paths = sys::CypherSystem_Paths();

    const auto base_mount_result = fs::CypherFileSystem_MountDirectory(
        "",
        paths.base_path,
        fs::CYPHER_FILESYSTEM_MOUNT_READ_ONLY,
        0u );

    if ( base_mount_result != fs::error_code_t::OK ) {
        CYPHER_LOG_ERROR( log::channel_t::FS, "filesystem base mount failed: base='%s', error=%s.", paths.base_path, fs::CypherFileSystem_ErrorDesc( base_mount_result ) );
        rc::CypherCommon_Errorf(
            fs::CypherFileSystem_ErrorCode( base_mount_result ),
            "CypherHost_Init: filesystem base mount failed: %s",
            fs::CypherFileSystem_ErrorDesc( base_mount_result ) );
        return error_code_t::ERR_INITIALIZING;
    }

    const auto write_path_result = fs::CypherFileSystem_SetWritePath( paths.user_path );
    if ( write_path_result != fs::error_code_t::OK ) {
        CYPHER_LOG_ERROR( log::channel_t::FS, "filesystem write path failed: user='%s', error=%s.", paths.user_path, fs::CypherFileSystem_ErrorDesc( write_path_result ) );
        rc::CypherCommon_Errorf(
            fs::CypherFileSystem_ErrorCode( write_path_result ),
            "CypherHost_Init: filesystem write path failed: %s",
            fs::CypherFileSystem_ErrorDesc( write_path_result ) );
        return error_code_t::ERR_INITIALIZING;
    }

    CYPHER_LOG_INFO( log::channel_t::FS, "filesystem mounted: base='%s', write='%s'.", paths.base_path, paths.user_path );
    return error_code_t::OK;
}

/*
================
CypherHost_RegisterBuiltinCvars
================
*/
error_code_t CypherHost_RegisterBuiltinCvars( void ) {
    struct builtin_cvar_t {
        const char *name;
        const char *default_value;
        cvar::flags_t flags;
    };

	    const builtin_cvar_t builtin_cvars[] = {
        { "r_width", "1280", cvar::CYPHER_CVAR_ARCHIVE },
        { "r_height", "720", cvar::CYPHER_CVAR_ARCHIVE },
        { "r_fullscreen", "0", cvar::CYPHER_CVAR_ARCHIVE },
        { "r_vsync", "1", cvar::CYPHER_CVAR_ARCHIVE },
        { "r_fov", "90", cvar::CYPHER_CVAR_ARCHIVE },
        { "r_near", "0.1", cvar::CYPHER_CVAR_ARCHIVE },
        { "r_far", "1000", cvar::CYPHER_CVAR_ARCHIVE },

        { "host_target_fps", "60", cvar::CYPHER_CVAR_ARCHIVE },
        { "host_timescale", "1.0", cvar::CYPHER_CVAR_ARCHIVE },
        { "host_max_delta_time", "0.25", cvar::CYPHER_CVAR_ARCHIVE },

        { "developer", "1", cvar::CYPHER_CVAR_ARCHIVE },
        { "con_show", "0", cvar::CYPHER_CVAR_ARCHIVE },

        { "sys_app_name", rc::COM_GAME_INFO.internal_name, cvar::CYPHER_CVAR_READONLY },
    };

    for ( const builtin_cvar_t &builtin_cvar : builtin_cvars ) {
        const auto result = cvar::CypherCVar_Register(
            builtin_cvar.name,
            builtin_cvar.default_value,
            builtin_cvar.flags );

        if ( result != cvar::error_code_t::OK ) {
            rc::CypherCommon_Errorf(
                cvar::CypherCVar_ErrorCode( result ),
                "CypherHost_Init: failed to register cvar '%s': %s",
                builtin_cvar.name,
                cvar::CypherCVar_ErrorDesc( result ) );
            return error_code_t::ERR_INITIALIZING;
        }
    }

    CYPHER_LOG_INFO( log::channel_t::CVAR, "registered %zu builtin cvars.", sizeof( builtin_cvars ) / sizeof( builtin_cvars[0] ) );
    return error_code_t::OK;
}

/*
================
CypherHost_RegisterBuiltinCommands
================
*/
error_code_t CypherHost_RegisterBuiltinCommands( state_t &host_state ) {
    struct builtin_command_t {
        const char *name;
        cmd::command_fn_t callback;
        void *extra_data;
        const char *description;
    };

    const builtin_command_t builtin_commands[] = {
        { "echo", CypherHost_CmdEcho, nullptr, "prints text to the engine console" },
        { "version", CypherHost_CmdVersion, nullptr, "prints engine and game version information" },
        { "quit", CypherHost_CmdQuit, &host_state, "requests engine shutdown." }
    };

    for ( const builtin_command_t &builtin_command : builtin_commands ) {
        const auto result = cmd::CypherCommand_Register(
            builtin_command.name,
            builtin_command.callback,
            builtin_command.extra_data,
            builtin_command.description );

        if ( result != cmd::error_code_t::OK ) {
            rc::CypherCommon_Errorf(
                cmd::CypherCommand_ErrorCode( result ),
                "CypherHost_Init: failed to register command '%s': %s",
                builtin_command.name,
                cmd::CypherCommand_ErrorDesc( result ) );
            return error_code_t::ERR_INITIALIZING;
        }
    }

    CYPHER_LOG_INFO( log::channel_t::CMD, "registered %zu builtin commands.", sizeof( builtin_commands ) / sizeof( builtin_commands[0] ) );
    return error_code_t::OK;
}

/*
================
CypherHost_LoadStartupConfig
================
*/
error_code_t CypherHost_LoadStartupConfig( void ) {
    const auto default_result = cfg::CypherConfig_LoadFile( "config/default.cfg", false );
    if ( default_result != cfg::error_code_t::OK ) {
        CYPHER_LOG_ERROR( log::channel_t::CFG, "default startup config failed: %s.", cfg::CypherConfig_ErrorDesc( default_result ) );
        rc::CypherCommon_Errorf(
            cfg::CypherConfig_ErrorCode( default_result ),
            "CypherHost_Init: default config load failed: %s",
            cfg::CypherConfig_ErrorDesc( default_result ) );
        return error_code_t::ERR_INITIALIZING;
    }

    const auto autoexec_result = cfg::CypherConfig_LoadAutoexec();
    if ( autoexec_result != cfg::error_code_t::OK ) {
        CYPHER_LOG_ERROR( log::channel_t::CFG, "autoexec startup config failed: %s.", cfg::CypherConfig_ErrorDesc( autoexec_result ) );
        rc::CypherCommon_Errorf(
            cfg::CypherConfig_ErrorCode( autoexec_result ),
            "CypherHost_Init: autoexec config load failed: %s",
            cfg::CypherConfig_ErrorDesc( autoexec_result ) );
        return error_code_t::ERR_INITIALIZING;
    }

    CYPHER_LOG_INFO( log::channel_t::CFG, "startup configs loaded." );
    return error_code_t::OK;
}

/*
================
CypherHost_ApplyCvarsToConfig
================
*/
error_code_t CypherHost_ApplyCvarsToConfig( state_t &host_state ) {
    host::window_config_t &window_config = host_state.config.window_config;

    const common::u32 width = cvar::CypherCVar_GetInt( "r_width" );
    const common::u32 height = cvar::CypherCVar_GetInt( "r_height" );
    const common::u32 target_fps = cvar::CypherCVar_GetInt( "host_target_fps" );

    if ( width != 0u ) {
        window_config.viewport.width = width;
    }

    if ( height != 0u ) {
        window_config.viewport.height = height;
    }

    if ( target_fps != 0u ) {
        window_config.target_fps = target_fps;
    }

    window_config.fullscreen = cvar::CypherCVar_GetBool( "r_fullscreen" );
    window_config.vsync = cvar::CypherCVar_GetBool( "r_vsync" );

    CYPHER_LOG_INFO( log::channel_t::HOST, "applied startup cvars: viewport=%ux%u, fullscreen=%u, vsync=%u, target_fps=%u.", window_config.viewport.width, window_config.viewport.height, window_config.fullscreen ? 1u : 0u, window_config.vsync ? 1u : 0u, window_config.target_fps );

    return error_code_t::OK;
}

/*
================
CypherHost_CreateWindow
================
*/
error_code_t CypherHost_CreateWindow( state_t &host_state ) 
{
    sys::window_desc_t window_description{};
     
    window_description.title        = host_state.config.window_config.title;
    window_description.width        = host_state.config.window_config.viewport.width;
    window_description.height       = host_state.config.window_config.viewport.height;
    window_description.fullscreen   = host_state.config.window_config.fullscreen;
    window_description.vsync        = host_state.config.window_config.vsync; 
    
    const auto window_result = sys::CypherSystem_CreateWindow( window_description, host_state.window );
    if ( window_result != sys::error_code_t::OK ) {
        CYPHER_LOG_ERROR( log::channel_t::HOST, "window creation failed: %s.", sys::CypherSystem_ErrorDesc( window_result ) );
        common::CypherCommon_Errorf( sys::CypherSystem_ErrorCode( window_result ), "CypherHost_CreateWindow: CypherSystem_CreateWindow failed: %s", sys::CypherSystem_ErrorDesc( window_result ) );
        return error_code_t::ERR_INITIALIZING;
    }
    
    return error_code_t::OK;
}

/*
================
CypherHost_InitRenderer
================
*/
error_code_t CypherHost_InitRenderer( state_t &host_state ) {
    const auto render_result = render::CypherRender_Init( host_state.window, host_state.config.window_config );
    if ( render_result != render::error_code_t::OK ) {
        CYPHER_LOG_ERROR( log::channel_t::RENDER, "renderer initialization failed." );
        rc::CypherCommon_Errorf(
            render::CypherRender_ErrorCode( render_result ),
            "CypherHost_Init: renderer initialization failed." );
        return error_code_t::ERR_INITIALIZING;
    }
    
    return error_code_t::OK;
}

/*
================
CypherHost_FinishInit
================
*/
error_code_t CypherHost_FinishInit( state_t &host_state ) {
    host_state.running = true;
    host_state.stage = stage_t::RUNNING;

    const common::com_f64 now = sys::CypherSystem_TimeNowSeconds();

    host_state.frame.current_time_seconds = now;
    host_state.frame.previous_time_seconds = now;

    CYPHER_LOG_INFO( log::channel_t::HOST, "%s startup complete.", common::COM_ENGINE_INFO.name );

    return error_code_t::OK;
}

/*
================
CypherHost_RequestShutdown
================
*/
void CypherHost_RequestShutdown( state_t &host_state )
{
    if ( host_state.stage == stage_t::SHUTDOWN ) {
        return ;
    }

    host_state.running = false;
    host_state.stage = stage_t::SHUTTINGDOWN;

    CYPHER_LOG_INFO( log::channel_t::HOST, "shutdown requested." );

    return ;
}

/*
================
CypherHost_Init

Main engine startup sequence.
================
*/
error_code_t CypherHost_Init( state_t &host_state ) {
    error_code_t result{};

    CypherHost_PrepareStateForInit( host_state );

    result = CypherHost_InitCoreEngineSystems( host_state );
    if ( result != error_code_t::OK ) {
        return result;
    }

    result = CypherHost_MountFileSystem();
    if ( result != error_code_t::OK ) {
        CypherHost_Shutdown( host_state );
        return result;
    }

    result = CypherHost_RegisterBuiltinCvars();
    if ( result != error_code_t::OK ) {
        CypherHost_Shutdown( host_state );
        return result;
    }

    result = CypherHost_RegisterBuiltinCommands( host_state );
    if ( result != error_code_t::OK ) {
        CypherHost_Shutdown( host_state );
        return result;
    }

    result = CypherHost_LoadStartupConfig();
    if ( result != error_code_t::OK ) {
        CypherHost_Shutdown( host_state );
        return result;
    }

    result = CypherHost_ApplyCvarsToConfig( host_state );
    if ( result != error_code_t::OK ) {
        CypherHost_Shutdown( host_state );
        return result;
    }
    
    result = CypherHost_CreateWindow( host_state );
    if ( result != error_code_t::OK ) {
        CypherHost_Shutdown( host_state );
        return result;
    }

    result = CypherHost_InitRenderer( host_state );
    if ( result != error_code_t::OK ) {
        CypherHost_Shutdown( host_state );
        return result;
    }

    result = CypherHost_FinishInit( host_state );
    if ( result != error_code_t::OK ) {
        CypherHost_Shutdown( host_state );
        return result;
    }

    return result;
}

/*
================
CypherHost_Shutdown
================
*/
void CypherHost_Shutdown( state_t &host_state ) {
    CYPHER_LOG_INFO( log::channel_t::HOST, "%s shutdown begin.", common::COM_ENGINE_INFO.name );

	host_state.running = false;
	host_state.stage = stage_t::SHUTDOWN;

    render::CypherRender_Shutdown();
    sys::CypherSystem_DestroyWindow( host_state.window );
    cfg::CypherConfig_Shutdown();
    cvar::CypherCVar_Shutdown();
    cmd::CypherCommand_Shutdown();
    fs::CypherFileSystem_Shutdown();
    CYPHER_LOG_INFO( log::channel_t::HOST, "%s shutdown complete.", common::COM_ENGINE_INFO.name );
    log::CypherLog_Shutdown();
    sys::CypherSystem_Shutdown();
}

/*
================
CypherHost_BeginFrame

Updates frame timing and opens the renderer frame.
================
*/
void CypherHost_BeginFrame( state_t &host_state ) {
	if ( host_state.stage == stage_t::SHUTDOWN ) {
		return;
	}

	frame_t &frame = host_state.frame;

	frame.previous_time_seconds = frame.current_time_seconds;
	frame.current_time_seconds = sys::CypherSystem_TimeNowSeconds();

    const common::f32 raw_delta_time_seconds = static_cast<common::f32>( frame.current_time_seconds - frame.previous_time_seconds );

    const common::f32 max_delta_time_seconds = cvar::CypherCVar_GetFloat( "host_max_delta_time" );
    const common::f32 timescale = cvar::CypherCVar_GetFloat( "host_timescale" );

    const common::f32 safe_max_delta_time_seconds = ( max_delta_time_seconds > 0.0f ) ? max_delta_time_seconds : 0.25f;
    const common::f32 safe_timescale = ( timescale >= 0.0f ) ? timescale : 1.0f;

    frame.delta_time_seconds = ( raw_delta_time_seconds > safe_max_delta_time_seconds )   ? safe_max_delta_time_seconds : raw_delta_time_seconds;

    frame.real_time_seconds += raw_delta_time_seconds;

	if ( host_state.stage == stage_t::RUNNING ) {
		frame.simulation_time_seconds += frame.delta_time_seconds * safe_timescale;
	}

	frame.index++;

	const auto render_result = render::CypherRender_BeginFrame( frame.delta_time_seconds );

	if ( render_result != render::error_code_t::OK ) {
		rc::CypherCommon_Errorf(
			render::CypherRender_ErrorCode( render_result ),
			"CypherHost_BeginFrame: renderer begin-frame failed." );

		host_state.running = false;
		host_state.stage = stage_t::SHUTTINGDOWN;
		return;
	}
}

/*
================
CypherHost_Update

Polls platform events and advances runtime systems.
================
*/
void CypherHost_Update( state_t &host_state ) {
	if ( host_state.stage != stage_t::RUNNING ) {
		return;
	}

	if ( !host_state.running ) {
		return;
	}

    sys::CypherSystem_PollWindowEvents( host_state.window );   
    
    if ( sys::CypherSystem_WindowShouldClose( host_state.window ) ) {
        CypherHost_RequestShutdown( host_state );
        return ;
    }
    
    /*
    Future order:
    CypherHost_UpdateInput -> CypherHost_UpdateConsole -> CypherHost_UpdateGame -> CypherHost_UpdateAudio.
     */
}

/*
================
CypherHost_Render
================
*/
void CypherHost_Render( state_t &host_state ) {
	if ( host_state.stage != stage_t::RUNNING || !host_state.running ) {
		return;
	}

	const auto render_result = render::CypherRender_RenderFrame();

	if ( render_result != render::error_code_t::OK ) {
		rc::CypherCommon_Errorf(
			render::CypherRender_ErrorCode( render_result ),
			"CypherHost_Render: renderer frame submission failed." );

		host_state.running = false;
		host_state.stage = stage_t::SHUTTINGDOWN;
	}
}

/*
================
CypherHost_EndFrame
================
*/
void CypherHost_EndFrame( state_t &host_state ) {
	if ( host_state.stage == stage_t::SHUTDOWN ) {
		return;
	}

	const auto render_result = render::CypherRender_EndFrame();

	if ( render_result != render::error_code_t::OK ) {
		rc::CypherCommon_Errorf(
			render::CypherRender_ErrorCode( render_result ),
			"CypherHost_EndFrame: renderer end-frame failed." );

		host_state.running = false;
		host_state.stage = stage_t::SHUTDOWN;
		return;
	}

	if ( host_state.stage == stage_t::SHUTTINGDOWN ) {
		host_state.running = false;
		host_state.stage = stage_t::SHUTDOWN;
	}
}

/*
================
CypherHost_IsRunning
================
*/
bool CypherHost_IsRunning( state_t &host_state ) {
	return host_state.running && ( host_state.stage != stage_t::SHUTTINGDOWN && host_state.stage != stage_t::SHUTDOWN );
}

}       // namespace cypher::engine::host
