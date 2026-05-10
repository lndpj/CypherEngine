/*======================================================================
   File: host_main.cpp
   Project: REAP
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
#include "rengine/cmd/cmd_main.h"
#include "rengine/cvar/cvar_main.h"
#include "rengine/rcommon/com_print.h"
#include "rengine/host/host_main.h"
#include "rengine/cfg/cfg_main.h"
#include "rengine/fs/fs_main.h"
#include "rengine/log/log_main.h"
#include "rengine/render/r_main.h"
#include "rengine/sys/sys_platform.h"

namespace rc = reap::rengine::rcommon;

namespace {

/*
================
Host Builtin Commands
================
*/
void Host_CmdEcho( void *extra_data, rc::u32 argc, char **argv ) {
    ( void )extra_data;
    for ( rc::u32 i = 1u; i < argc; ++i ) {
        rc::Com_Printf( "%s%s", argv[i], ( i + 1u < argc ) ? " " : "\n" );
    }
    if ( argc <= 1u ) {
        rc::Com_Printf( "\n" );
    }
}

void Host_CmdVersion( void *extra_data, rc::u32 argc, char **argv ) {
    ( void )extra_data;
    ( void )argc;
    ( void )argv;

    const rc::com_version_t &engine_version = rc::COM_ENGINE_INFO.version;
    const rc::com_version_t &game_version = rc::COM_GAME_INFO.version;

    rc::Com_Printf(
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

void Host_CmdQuit( void *extra_data, rc::u32 argc, char **argv ) {
    ( void )argc;
    ( void )argv;

    reap::rengine::host::host_state_t *host_state = static_cast<reap::rengine::host::host_state_t *>( extra_data );

    if ( host_state == nullptr ) {
        return ;
    }

    Host_RequestShutdown( *host_state );
}

}       // namespace

namespace reap::rengine::host {

/*
================
Host_PrepareStateForInit
================
*/
void Host_PrepareStateForInit( host_state_t &host_state ) {
    host_state.stage = host_stage_t::INITIALIZING;
    host_state.running = false;
    host_state.has_focus = true;
    host_state.frame = {};
}

/*
================
Host_InitCoreEngineSystems

Brings up low-level systems in dependency order.
================
*/
host_error_code_t Host_InitCoreEngineSystems( host_state_t &host_state ) {
    sys::sys_init_info_t sys_info {
        .argc = host_state.config.argc,
        .argv = host_state.config.argv,
        .app_name = rcommon::COM_GAME_INFO.internal_name,
        .organization_name = rcommon::COM_GAME_INFO.organization_name
    };

    const auto sys_result = sys::Sys_Init( sys_info );
    if ( sys_result != sys::sys_error_code_t::OK ) {
        rcommon::Com_Errorf( Sys_ErrorCode( sys_result ) , "Host_Init: Sys_Init failed: %s", sys::Sys_ErrorDesc( sys_result ) );

        host_state.running = false;
        host_state.stage = host_stage_t::SHUTDOWN;
        return host_error_code_t::ERR_INITIALIZING;
    }

    const auto log_result = log::Log_Init();
    if ( log_result != log::log_error_code_t::OK ) {
        rcommon::Com_Errorf( log::Log_ErrorCode( log_result ), "Host_Init: Log_Init failed: %s", log::Log_ErrorDesc( log_result ) );

        sys::Sys_Shutdown();

        host_state.running = false;
        host_state.stage = host_stage_t::SHUTDOWN;
        return host_error_code_t::ERR_INITIALIZING;
    }

    const auto fs_result = fs::FS_Init();
    if( fs_result != fs::fs_error_code_t::OK ) {
        rcommon::Com_Errorf( FS_ErrorCode( fs_result ), "Host_Init: FS_Init failed: %s", fs::FS_ErrorDesc( fs_result ) );
        log::Log_Shutdown();
        sys::Sys_Shutdown();

        host_state.running = false;
        host_state.stage = host_stage_t::SHUTDOWN;
        return host_error_code_t::ERR_INITIALIZING;
    }
    const auto cmd_result = cmd::Cmd_Init();
    if ( cmd_result != cmd::cmd_error_code_t::OK )
    {
        rcommon::Com_Errorf( Cmd_ErrorCode( cmd_result ), "Host_Init: Cmd_Init failed: %s", Cmd_ErrorDesc( cmd_result ) );

        fs::FS_Shutdown();
        log::Log_Shutdown();
        sys::Sys_Shutdown();

        host_state.running = false;
        host_state.stage = host_stage_t::SHUTDOWN;
        return host_error_code_t::ERR_INITIALIZING;
    }

    const auto cvar_result = cvar::Cvar_Init();

    if ( cvar_result != cvar::cvar_error_code_t::OK )
    {
        rcommon::Com_Errorf( Cvar_ErrorCode( cvar_result ), "Host_Init: Cvar_Init failed: %s", cvar::Cvar_ErrorDesc( cvar_result ) );

        cmd::Cmd_Shutdown();
        fs::FS_Shutdown();
        log::Log_Shutdown();
        sys::Sys_Shutdown();

        host_state.running = false;
        host_state.stage = host_stage_t::SHUTDOWN;
        return host_error_code_t::ERR_INITIALIZING;
    }

    const auto cfg_result = cfg::Cfg_Init();

    if ( cfg_result != cfg::cfg_error_code_t::OK )
    {
        rcommon::Com_Errorf( Cfg_ErrorCode( cfg_result ), "Host_Init: Cfg_Init failed: %s", cfg::Cfg_ErrorDesc( cfg_result ) );

        cvar::Cvar_Shutdown();
        cmd::Cmd_Shutdown();
        fs::FS_Shutdown();
        log::Log_Shutdown();
        sys::Sys_Shutdown();

        host_state.running = false;
        host_state.stage = host_stage_t::SHUTDOWN;
        return host_error_code_t::ERR_INITIALIZING;
    }

    return host_error_code_t::OK;
}

/*
================
Host_MountFileSystem
================
*/
host_error_code_t Host_MountFileSystem( void ) {
    const sys::sys_paths_t &paths = sys::Sys_Paths();

    const auto base_mount_result = fs::FS_MountDirectory(
        "",
        paths.base_path,
        fs::FS_MOUNT_READ_ONLY,
        0u );

    if ( base_mount_result != fs::fs_error_code_t::OK ) {
        rc::Com_Errorf(
            fs::FS_ErrorCode( base_mount_result ),
            "Host_Init: filesystem base mount failed: %s",
            fs::FS_ErrorDesc( base_mount_result ) );
        return host_error_code_t::ERR_INITIALIZING;
    }

    const auto write_path_result = fs::FS_SetWritePath( paths.user_path );
    if ( write_path_result != fs::fs_error_code_t::OK ) {
        rc::Com_Errorf(
            fs::FS_ErrorCode( write_path_result ),
            "Host_Init: filesystem write path failed: %s",
            fs::FS_ErrorDesc( write_path_result ) );
        return host_error_code_t::ERR_INITIALIZING;
    }

    return host_error_code_t::OK;
}

/*
================
Host_RegisterBuiltinCvars
================
*/
host_error_code_t Host_RegisterBuiltinCvars( void ) {
    struct builtin_cvar_t {
        const char *name;
        const char *default_value;
        cvar::cvar_flags_t flags;
    };

    const builtin_cvar_t builtin_cvars[] = {
        { "r_width", "1280", cvar::CVAR_ARCHIVE },
        { "r_height", "720", cvar::CVAR_ARCHIVE },
        { "r_fullscreen", "0", cvar::CVAR_ARCHIVE },
        { "r_vsync", "1", cvar::CVAR_ARCHIVE },
        { "r_fov", "90", cvar::CVAR_ARCHIVE },
        { "r_near", "0.1", cvar::CVAR_ARCHIVE },
        { "r_far", "1000", cvar::CVAR_ARCHIVE },

        { "host_target_fps", "60", cvar::CVAR_ARCHIVE },
        { "host_timescale", "1.0", cvar::CVAR_ARCHIVE },
        { "host_max_delta_time", "0.25", cvar::CVAR_ARCHIVE },

        { "developer", "1", cvar::CVAR_ARCHIVE },
        { "con_show", "0", cvar::CVAR_ARCHIVE },

        { "sys_app_name", rc::COM_GAME_INFO.internal_name, cvar::CVAR_READONLY },
    };

    for ( const builtin_cvar_t &builtin_cvar : builtin_cvars ) {
        const auto result = cvar::Cvar_Register(
            builtin_cvar.name,
            builtin_cvar.default_value,
            builtin_cvar.flags );

        if ( result != cvar::cvar_error_code_t::OK ) {
            rc::Com_Errorf(
                cvar::Cvar_ErrorCode( result ),
                "Host_Init: failed to register cvar '%s': %s",
                builtin_cvar.name,
                cvar::Cvar_ErrorDesc( result ) );
            return host_error_code_t::ERR_INITIALIZING;
        }
    }

    return host_error_code_t::OK;
}

/*
================
Host_RegisterBuiltinCommands
================
*/
host_error_code_t Host_RegisterBuiltinCommands( host_state_t &host_state ) {
    struct builtin_command_t {
        const char *name;
        cmd::cmd_fn_t callback;
        void *extra_data;
        const char *description;
    };

    const builtin_command_t builtin_commands[] = {
        { "echo", Host_CmdEcho, nullptr, "prints text to the engine console" },
        { "version", Host_CmdVersion, nullptr, "prints engine and game version information" },
        { "quit", Host_CmdQuit, &host_state, "requests engine shutdown." }
    };

    for ( const builtin_command_t &builtin_command : builtin_commands ) {
        const auto result = cmd::Cmd_Register(
            builtin_command.name,
            builtin_command.callback,
            builtin_command.extra_data,
            builtin_command.description );

        if ( result != cmd::cmd_error_code_t::OK ) {
            rc::Com_Errorf(
                cmd::Cmd_ErrorCode( result ),
                "Host_Init: failed to register command '%s': %s",
                builtin_command.name,
                cmd::Cmd_ErrorDesc( result ) );
            return host_error_code_t::ERR_INITIALIZING;
        }
    }

    return host_error_code_t::OK;
}

/*
================
Host_LoadStartupConfig
================
*/
host_error_code_t Host_LoadStartupConfig( void ) {
    const auto default_result = cfg::Cfg_LoadFile( "config/default.cfg", false );
    if ( default_result != cfg::cfg_error_code_t::OK ) {
        rc::Com_Errorf(
            cfg::Cfg_ErrorCode( default_result ),
            "Host_Init: default config load failed: %s",
            cfg::Cfg_ErrorDesc( default_result ) );
        return host_error_code_t::ERR_INITIALIZING;
    }

    const auto autoexec_result = cfg::Cfg_LoadAutoexec();
    if ( autoexec_result != cfg::cfg_error_code_t::OK ) {
        rc::Com_Errorf(
            cfg::Cfg_ErrorCode( autoexec_result ),
            "Host_Init: autoexec config load failed: %s",
            cfg::Cfg_ErrorDesc( autoexec_result ) );
        return host_error_code_t::ERR_INITIALIZING;
    }

    return host_error_code_t::OK;
}

/*
================
Host_ApplyCvarsToConfig
================
*/
host_error_code_t Host_ApplyCvarsToConfig( host_state_t &host_state ) {
    host::window_config_t &window_config = host_state.config.window_config;

    const rcommon::u32 width = cvar::Cvar_GetInt( "r_width" );
    const rcommon::u32 height = cvar::Cvar_GetInt( "r_height" );
    const rcommon::u32 target_fps = cvar::Cvar_GetInt( "host_target_fps" );

    if ( width != 0u ) {
        window_config.viewport.width = width;
    }

    if ( height != 0u ) {
        window_config.viewport.height = height;
    }

    if ( target_fps != 0u ) {
        window_config.target_fps = target_fps;
    }

    window_config.fullscreen = cvar::Cvar_GetBool( "r_fullscreen" );
    window_config.vsync = cvar::Cvar_GetBool( "r_vsync" );

    return host_error_code_t::OK;
}

/*
================
Host_CreateWindow
================
*/
host_error_code_t Host_CreateWindow( host_state_t &host_state ) 
{
    sys::sys_window_desc_t window_description{};
     
    window_description.title        = host_state.config.window_config.title;
    window_description.width        = host_state.config.window_config.viewport.width;
    window_description.height       = host_state.config.window_config.viewport.height;
    window_description.fullscreen   = host_state.config.window_config.fullscreen;
    window_description.vsync        = host_state.config.window_config.vsync; 
    
    const auto window_result = sys::Sys_CreateWindow( window_description, host_state.window );
    if ( window_result != sys::sys_error_code_t::OK ) {
        rcommon::Com_Errorf( sys::Sys_ErrorCode( window_result ), "Host_CreateWindow: Sys_CreateWindow failed: %s", sys::Sys_ErrorDesc( window_result ) );
        return host_error_code_t::ERR_INITIALIZING;
    }
    
    return host_error_code_t::OK;
}

/*
================
Host_InitRenderer
================
*/
host_error_code_t Host_InitRenderer( host_state_t &host_state ) {
    const auto render_result = render::R_Init( host_state.window, host_state.config.window_config );
    if ( render_result != render::r_error_code_t::OK ) {
        rc::Com_Errorf(
            render::R_ErrorCode( render_result ),
            "Host_Init: renderer initialization failed." );
        return host_error_code_t::ERR_INITIALIZING;
    }
    
    return host_error_code_t::OK;
}

/*
================
Host_FinishInit
================
*/
host_error_code_t Host_FinishInit( host_state_t &host_state ) {
    host_state.running = true;
    host_state.stage = host_stage_t::RUNNING;

    const rcommon::com_f64 now = sys::Sys_TimeNowSeconds();

    host_state.frame.current_time_seconds = now;
    host_state.frame.previous_time_seconds = now;

    return host_error_code_t::OK;
}

/*
================
Host_RequestShutdown
================
*/
void Host_RequestShutdown( host_state_t &host_state )
{
    if ( host_state.stage == host_stage_t::SHUTDOWN ) {
        return ;
    }

    host_state.running = false;
    host_state.stage = host_stage_t::SHUTTINGDOWN;

    return ;
}

/*
================
Host_Init

Main engine startup sequence.
================
*/
host_error_code_t Host_Init( host_state_t &host_state ) {
    host_error_code_t result{};

    Host_PrepareStateForInit( host_state );

    result = Host_InitCoreEngineSystems( host_state );
    if ( result != host_error_code_t::OK ) {
        return result;
    }

    result = Host_MountFileSystem();
    if ( result != host_error_code_t::OK ) {
        Host_Shutdown( host_state );
        return result;
    }

    result = Host_RegisterBuiltinCvars();
    if ( result != host_error_code_t::OK ) {
        Host_Shutdown( host_state );
        return result;
    }

    result = Host_RegisterBuiltinCommands( host_state );
    if ( result != host_error_code_t::OK ) {
        Host_Shutdown( host_state );
        return result;
    }

    result = Host_LoadStartupConfig();
    if ( result != host_error_code_t::OK ) {
        Host_Shutdown( host_state );
        return result;
    }

    result = Host_ApplyCvarsToConfig( host_state );
    if ( result != host_error_code_t::OK ) {
        Host_Shutdown( host_state );
        return result;
    }
    
    result = Host_CreateWindow( host_state );
    if ( result != host_error_code_t::OK ) {
        Host_Shutdown( host_state );
        return result;
    }

    result = Host_InitRenderer( host_state );
    if ( result != host_error_code_t::OK ) {
        Host_Shutdown( host_state );
        return result;
    }

    result = Host_FinishInit( host_state );
    if ( result != host_error_code_t::OK ) {
        Host_Shutdown( host_state );
        return result;
    }

    return result;
}

/*
================
Host_Shutdown
================
*/
void Host_Shutdown( host_state_t &host_state ) {
	host_state.running = false;
	host_state.stage = host_stage_t::SHUTDOWN;

    render::R_Shutdown();
    sys::Sys_DestroyWindow( host_state.window );
    cfg::Cfg_Shutdown();
    cvar::Cvar_Shutdown();
    cmd::Cmd_Shutdown();
    fs::FS_Shutdown();
    log::Log_Shutdown();
    sys::Sys_Shutdown();
}

/*
================
Host_BeginFrame

Updates frame timing and opens the renderer frame.
================
*/
void Host_BeginFrame( host_state_t &host_state ) {
	if ( host_state.stage == host_stage_t::SHUTDOWN ) {
		return;
	}

	frame_t &frame = host_state.frame;

	frame.previous_time_seconds = frame.current_time_seconds;
	frame.current_time_seconds = sys::Sys_TimeNowSeconds();

    const rcommon::f32 raw_delta_time_seconds = static_cast<rcommon::f32>( frame.current_time_seconds - frame.previous_time_seconds );

    const rcommon::f32 max_delta_time_seconds = cvar::Cvar_GetFloat( "host_max_delta_time" );
    const rcommon::f32 timescale = cvar::Cvar_GetFloat( "host_timescale" );

    const rcommon::f32 safe_max_delta_time_seconds = ( max_delta_time_seconds > 0.0f ) ? max_delta_time_seconds : 0.25f;
    const rcommon::f32 safe_timescale = ( timescale >= 0.0f ) ? timescale : 1.0f;

    frame.delta_time_seconds = ( raw_delta_time_seconds > safe_max_delta_time_seconds )   ? safe_max_delta_time_seconds : raw_delta_time_seconds;

    frame.real_time_seconds += raw_delta_time_seconds;

	if ( host_state.stage == host_stage_t::RUNNING ) {
		frame.simulation_time_seconds += frame.delta_time_seconds * safe_timescale;
	}

	frame.index++;

	const auto render_result = render::R_BeginFrame( frame.delta_time_seconds );

	if ( render_result != render::r_error_code_t::OK ) {
		rc::Com_Errorf(
			render::R_ErrorCode( render_result ),
			"Host_BeginFrame: renderer begin-frame failed." );

		host_state.running = false;
		host_state.stage = host_stage_t::SHUTTINGDOWN;
		return;
	}
}

/*
================
Host_Update

Polls platform events and advances runtime systems.
================
*/
void Host_Update( host_state_t &host_state ) {
	if ( host_state.stage != host_stage_t::RUNNING ) {
		return;
	}

	if ( !host_state.running ) {
		return;
	}

    sys::Sys_PollWindowEvents( host_state.window );   
    
    if ( sys::Sys_WindowShouldClose( host_state.window ) ) {
        Host_RequestShutdown( host_state );
        return ;
    }
    
    /*
    Future order:
    Host_UpdateInput -> Host_UpdateConsole -> Host_UpdateGame -> Host_UpdateAudio.
     */
}

/*
================
Host_Render
================
*/
void Host_Render( host_state_t &host_state ) {
	if ( host_state.stage != host_stage_t::RUNNING || !host_state.running ) {
		return;
	}

	const auto render_result = render::R_RenderFrame();

	if ( render_result != render::r_error_code_t::OK ) {
		rc::Com_Errorf(
			render::R_ErrorCode( render_result ),
			"Host_Render: renderer frame submission failed." );

		host_state.running = false;
		host_state.stage = host_stage_t::SHUTTINGDOWN;
	}
}

/*
================
Host_EndFrame
================
*/
void Host_EndFrame( host_state_t &host_state ) {
	if ( host_state.stage == host_stage_t::SHUTDOWN ) {
		return;
	}

	const auto render_result = render::R_EndFrame();

	if ( render_result != render::r_error_code_t::OK ) {
		rc::Com_Errorf(
			render::R_ErrorCode( render_result ),
			"Host_EndFrame: renderer end-frame failed." );

		host_state.running = false;
		host_state.stage = host_stage_t::SHUTDOWN;
		return;
	}

	if ( host_state.stage == host_stage_t::SHUTTINGDOWN ) {
		host_state.running = false;
		host_state.stage = host_stage_t::SHUTDOWN;
	}
}

/*
================
Host_IsRunning
================
*/
bool Host_IsRunning( host_state_t &host_state ) {
	return host_state.running && ( host_state.stage != host_stage_t::SHUTTINGDOWN && host_state.stage != host_stage_t::SHUTDOWN );
}

}       // namespace reap::rengine::host
