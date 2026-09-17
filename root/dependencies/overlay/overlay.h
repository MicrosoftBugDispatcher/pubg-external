#pragma once

#include <dependencies/includes.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dwmapi.h>
#include "font/fonts.hxx"
#include <src/utility/global/global.cuh>
#include <src/cheat/esp/esp.cuh>

inline ID3D11Device*            d3d_device        = nullptr;
inline ID3D11DeviceContext*     d3d_device_ctx    = nullptr;
inline IDXGISwapChain*          d3d_swap_chain    = nullptr;
inline ID3D11RenderTargetView*  d3d_render_target = nullptr;
inline HWND                     window_handle     = nullptr;
inline HWND                     game_hwnd         = nullptr;

inline bool show_menu       = false;
inline bool insert_was_down = false;

inline auto refresh_game_hwnd( ) -> void
{
    if ( game_hwnd && ::IsWindow( game_hwnd ) )
        return;
    game_hwnd = ::FindWindowA( nullptr , "Rust" );
}

inline auto rust_allows_draw( ) -> bool
{
    refresh_game_hwnd( );
    const bool rust_alive =
        game_hwnd &&
        ::IsWindow( game_hwnd ) &&
        !::IsIconic( game_hwnd );
    if ( !rust_alive )
        return false;

    const HWND foreground = ::GetForegroundWindow( );
    const bool rust_focused =
        foreground == game_hwnd ||
        ::GetAncestor( foreground , GA_ROOT ) == game_hwnd;
    const bool overlay_focused =
        window_handle &&
        foreground == window_handle;

    return rust_focused || ( show_menu && overlay_focused );
}

using namespace ImGui;

namespace overlay
{
    class c_overlay
    {
    public:
        auto setup( HWND hwnd ) -> bool
        {
            if ( !hwnd || !IsWindow( hwnd ) )
                return false;

            window_handle = hwnd;
            return init_imgui( );
        }

        auto init_imgui( ) -> bool
        {
            RECT rc{ };
            GetClientRect( window_handle , &rc );
            const UINT w = rc.right  > 0 ? static_cast<UINT>( rc.right  ) : static_cast<UINT>( GetSystemMetrics( SM_CXSCREEN ) );
            const UINT h = rc.bottom > 0 ? static_cast<UINT>( rc.bottom ) : static_cast<UINT>( GetSystemMetrics( SM_CYSCREEN ) );

            DXGI_SWAP_CHAIN_DESC desc{ };
            desc.BufferCount                        = 2;
            desc.BufferDesc.Width                   = w;
            desc.BufferDesc.Height                  = h;
            desc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.BufferDesc.RefreshRate.Numerator   = 60;
            desc.BufferDesc.RefreshRate.Denominator = 1;
            desc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            desc.OutputWindow                       = window_handle;
            desc.SampleDesc.Count                   = 1;
            desc.Windowed                           = TRUE;
            desc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
            desc.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

            D3D_FEATURE_LEVEL feat_level;
            const D3D_FEATURE_LEVEL feat_arr[ 1 ] = { D3D_FEATURE_LEVEL_11_0 };
            HRESULT hr = D3D11CreateDeviceAndSwapChain(
                nullptr , D3D_DRIVER_TYPE_HARDWARE , nullptr , 0 ,
                feat_arr , 1 , D3D11_SDK_VERSION , &desc ,
                &d3d_swap_chain , &d3d_device , &feat_level , &d3d_device_ctx );

            if ( FAILED( hr ) || !d3d_swap_chain || !d3d_device || !d3d_device_ctx )
                return false;

            ID3D11Texture2D* back_buf = nullptr;
            d3d_swap_chain->GetBuffer( 0 , IID_PPV_ARGS( &back_buf ) );
            if ( !back_buf )
                return false;

            d3d_device->CreateRenderTargetView( back_buf , nullptr , &d3d_render_target );
            back_buf->Release( );
            if ( !d3d_render_target )
                return false;

            IMGUI_CHECKVERSION( );
            ImGui::CreateContext( );
            ImGuiIO& io = ImGui::GetIO( );
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.DisplaySize = ImVec2( static_cast<float>( w ) , static_cast<float>( h ) );

            ImGuiStyle& style = ImGui::GetStyle( );
            style.AntiAliasedLines = false;
            style.AntiAliasedLinesUseTex = false;
            style.AntiAliasedFill = false;

            const unsigned int freetype_flags =
                ImGuiFreeTypeLoaderFlags_MonoHinting | ImGuiFreeTypeLoaderFlags_Monochrome;
            io.Fonts->SetFontLoader( ImGuiFreeType::GetFontLoader( ) );
            io.Fonts->FontLoaderFlags = freetype_flags;

            ImFontConfig mono_cfg;
            mono_cfg.FontLoaderFlags = freetype_flags;
            mono_cfg.OversampleH = 1;
            mono_cfg.OversampleV = 1;
            mono_cfg.PixelSnapH = true;
            mono_cfg.FontDataOwnedByAtlas = false;

            global::fonts::menu = io.Fonts->AddFontFromMemoryTTF(
                ( void* )tahoma_hex , ( int )sizeof( tahoma_hex ) , 13.f , &mono_cfg );
            if ( global::fonts::menu )
            {
                for ( int size = 6; size <= 17; ++size )
                    global::fonts::menu->GetFontBaked( ( float )size );
            }
            io.FontDefault = global::fonts::menu;

            global::fonts::pixel = io.Fonts->AddFontFromMemoryTTF(
                ( void* )font_sp7 , ( int )sizeof( font_sp7 ) , 10.f , &mono_cfg );

            ImFontConfig tahoma_cfg;
            tahoma_cfg.FontLoaderFlags = 0;
            tahoma_cfg.OversampleH = 3;
            tahoma_cfg.OversampleV = 3;
            tahoma_cfg.PixelSnapH = false;
            tahoma_cfg.FontDataOwnedByAtlas = false;

            global::fonts::name = io.Fonts->AddFontFromMemoryTTF(
                ( void* )tahoma_hex , ( int )sizeof( tahoma_hex ) , 13.f , &tahoma_cfg );
            global::fonts::esp = io.Fonts->AddFontFromMemoryTTF(
                ( void* )Tahoma_Bold , ( int )sizeof( Tahoma_Bold ) , 13.f , &tahoma_cfg );

            if ( !global::fonts::name )
                global::fonts::name = global::fonts::esp ? global::fonts::esp : global::fonts::menu;
            if ( !global::fonts::esp )
                global::fonts::esp = global::fonts::name ? global::fonts::name : global::fonts::menu;
            if ( !global::fonts::pixel )
                global::fonts::pixel = global::fonts::esp;
            if ( !global::fonts::menu )
                global::fonts::menu = global::fonts::name;

            ImGui_ImplWin32_Init( window_handle );
            ImGui_ImplDX11_Init( d3d_device , d3d_device_ctx );

            return true;
        }

        auto overlay( ) -> bool
        {
            MSG msg = { NULL };
            while ( msg.message != WM_QUIT )
            {
                while ( PeekMessageA( &msg , NULL , 0 , 0 , PM_REMOVE ) )
                {
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                    if ( msg.message == WM_QUIT )
                        break;
                }
                if ( msg.message == WM_QUIT )
                    break;

                ImGuiIO& io   = ImGui::GetIO( );
                io.DeltaTime  = 1.0f / 60.0f;

                POINT p;
                GetCursorPos( &p );
                io.MousePos.x = static_cast<float>( p.x );
                io.MousePos.y = static_cast<float>( p.y );
                io.MouseDown [ 0 ] = ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0;

                bool insert_now = ( GetAsyncKeyState( VK_INSERT ) & 0x8000 ) != 0;
                if ( insert_now && !insert_was_down )
                {
                    show_menu = !show_menu;
                    SetWindowLongA( window_handle , GWL_EXSTYLE , show_menu
                        ? ( WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW )
                        : ( WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED ) );
                }
                insert_was_down = insert_now;

                const bool allow_draw = rust_allows_draw( );
                if ( !allow_draw && !show_menu )
                {
                    const float cc[ 4 ] = { 0.f, 0.f, 0.f, 0.f };
                    d3d_device_ctx->OMSetRenderTargets( 1 , &d3d_render_target , nullptr );
                    d3d_device_ctx->ClearRenderTargetView( d3d_render_target , cc );
                    d3d_swap_chain->Present( 1 , 0 );
                    std::this_thread::sleep_for( std::chrono::milliseconds( 16 ) );
                    continue;
                }

                this->draw_frame( );
                d3d_swap_chain->Present( 1 , 0 );
            }

            ImGui_ImplDX11_Shutdown( );
            ImGui_ImplWin32_Shutdown( );
            ImGui::DestroyContext( );
            return true;
        }

        auto draw_frame( ) -> vi
        {
            ImGui_ImplDX11_NewFrame( );
            ImGui_ImplWin32_NewFrame( );
            ImGui::NewFrame( );

            const bool allow_draw = rust_allows_draw( );

            if ( show_menu )
            {
                ImGui::SetNextWindowSize( ImVec2( 820 , 520 ) , ImGuiCond_FirstUseEver );
                if ( ImGui::Begin( "rust-module" , nullptr ,
                                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize ) )
                {
                    float sidebar_width = 120.0f;

                    ImGui::BeginChild( "##sidebar" , ImVec2( sidebar_width , 0 ) , true );
                    const char* tabs [ ] = { "combat", "visuals", "weapon", "misc" };
                    static int selected_tab = 0;
                    for ( int i = 0; i < 4; ++i )
                    {
                        bool sel = ( selected_tab == i );
                        if ( sel ) ImGui::PushStyleColor( ImGuiCol_Text , ImVec4( 1.f , 1.f , 1.f , 1.f ) );
                        ImGui::SetCursorPosX( ( sidebar_width - 90.f ) * 0.5f );
                        if ( ImGui::Selectable( tabs [ i ] , sel , 0 , ImVec2( 90 , 18 ) ) )
                            selected_tab = i;
                        if ( sel ) ImGui::PopStyleColor( );
                    }
                    ImGui::EndChild( );

                    ImGui::SameLine( );
                    ImGui::BeginChild( "##content" , ImVec2( 0 , 0 ) , true );
                    ImGui::Text( "features > %s" , tabs [ selected_tab ] );
                    ImGui::Separator( );
                    ImGui::Spacing( );

                    
                    if ( selected_tab == 0 )
                    {
                        ImGui::Columns( 2 , "##combat_cols" , false );
                        {
                            ImGui::BeginChild( "##combat_left" , ImVec2( 0 , 0 ) , true );

                            ImGui::Checkbox( "enabled" , &global::aimbot::enabled );
                            ImGui::Checkbox( "silent aim" , &global::aimbot::silent );
                            ImGui::Checkbox( "prediction" , &global::aimbot::prediction );

                            ImGui::Spacing( );
                            ImGui::PushItemWidth( -FLT_MIN );

                            ImGui::SliderFloat( "##smooth" , &global::aimbot::smoothness , 1.f , 20.f , "%.1f" );
                            ImGui::SameLine( ); ImGui::Text( "smoothness" );

                            ImGui::SliderFloat( "##fov" , &global::aimbot::fov , 10.f , 500.f , "%.0fpx" );
                            ImGui::SameLine( ); ImGui::Text( "fov (px)" );

                            ImGui::SliderFloat( "##maxdist" , &global::aimbot::max_distance , 50.f , 500.f , "%.0fm" );
                            ImGui::SameLine( ); ImGui::Text( "max distance" );

                            ImGui::PopItemWidth( );

                            ImGui::EndChild( );
                        }
                        ImGui::NextColumn( );
                        {
                            ImGui::BeginChild( "##combat_right" , ImVec2( 0 , 0 ) , true );
                            ImGui::Text( "key: RMB" );
                            ImGui::EndChild( );
                        }
                        ImGui::Columns( 1 );
                    }

                    
                    else if ( selected_tab == 1 )
                    {
                        ImGui::Columns( 2 , "##vis_cols" , false );
                        {
                            ImGui::BeginChild( "##vis_left" , ImVec2( 0 , 0 ) , true );

                            ImGui::Checkbox( "draw esp" , &global::esp::draw );
                            ImGui::Separator( );

                            ImGui::Checkbox( "box" , &global::esp::box );
                            if ( global::esp::box )
                            {
                                ImGui::PushItemWidth( 100 );
                                static const char* box_types [ ] = { "full", "corner" };
                                ImGui::Combo( "##btype" , &global::esp::box_type , box_types , 2 );
                                ImGui::SameLine( ); ImGui::Text( "style" );
                                ImGui::PopItemWidth( );
                                ImGui::Checkbox( "box outline" , &global::esp::box_outline );
                                ImGui::Checkbox( "box filled" , &global::esp::box_filled );
                                ImGui::PushItemWidth( 100 );
                                ImGui::SliderInt( "##bthick" , &global::esp::box_thick , 1 , 4 );
                                ImGui::SameLine( ); ImGui::Text( "thick" );
                                ImGui::PopItemWidth( );
                            }

                            ImGui::Checkbox( "health bar" , &global::esp::health_bar );
                            if ( global::esp::health_bar )
                            {
                                ImGui::PushItemWidth( 100 );
                                static const char* hp_sides [ ] = { "left", "right", "top", "bottom" };
                                ImGui::Combo( "##hpside" , &global::esp::health_side , hp_sides , 4 );
                                ImGui::SameLine( ); ImGui::Text( "side" );
                                static const char* hp_modes [ ] = { "static", "gradient", "dynamic" };
                                ImGui::Combo( "##hpmode" , &global::esp::health_mode , hp_modes , 3 );
                                ImGui::SameLine( ); ImGui::Text( "mode" );
                                ImGui::SliderFloat( "##hpgap" , &global::esp::health_gap , 0.f , 12.f , "%.0f" );
                                ImGui::SameLine( ); ImGui::Text( "gap" );
                                ImGui::SliderFloat( "##hpw" , &global::esp::health_width , 1.f , 6.f , "%.0f" );
                                ImGui::SameLine( ); ImGui::Text( "width" );
                                ImGui::PopItemWidth( );
                            }

                            ImGui::Checkbox( "armor bar"  , &global::esp::armor_bar  );
                            if ( global::esp::armor_bar )
                            {
                                ImGui::PushItemWidth( 100 );
                                ImGui::SliderFloat( "##agap" , &global::esp::armor_gap , 0.f , 12.f , "%.0f" );
                                ImGui::SameLine( ); ImGui::Text( "gap" );
                                ImGui::SliderFloat( "##aw" , &global::esp::armor_width , 1.f , 6.f , "%.0f" );
                                ImGui::SameLine( ); ImGui::Text( "width" );
                                ImGui::PopItemWidth( );
                            }
                            ImGui::Checkbox( "skeleton"   , &global::esp::skeleton   );
                            if ( global::esp::skeleton )
                            {
                                ImGui::Checkbox( "skel outline" , &global::esp::skeleton_outline );
                                ImGui::PushItemWidth( 100 );
                                ImGui::SliderInt( "##sthick" , &global::esp::skel_thick , 1 , 4 );
                                ImGui::SameLine( ); ImGui::Text( "thick" );
                                ImGui::PopItemWidth( );
                            }
                            ImGui::Checkbox( "name"       , &global::esp::name       );
                            ImGui::Checkbox( "distance"   , &global::esp::distance   );
                            ImGui::Checkbox( "weapon"     , &global::esp::weapon     );
                            ImGui::Checkbox( "oof arrows" , &global::esp::oof_arrows );

                            ImGui::EndChild( );
                        }
                        ImGui::NextColumn( );
                        {
                            ImGui::BeginChild( "##vis_right" , ImVec2( 0 , 0 ) , true );

                            ImGui::Checkbox( "skip sleepers" , &global::esp::skip_sleepers );
                            ImGui::Checkbox( "skip team"     , &global::esp::skip_team     );
                            ImGui::Separator( );
                            ImGui::Text( "colors" );
                            ImGui::Spacing( );

                            const ImGuiColorEditFlags cf = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar;

                            ImGui::ColorEdit4( "##boxcol" , global::esp::box_color , cf );
                            ImGui::SameLine( ); ImGui::Text( "box vis" );
                            ImGui::ColorEdit4( "##boxcol1" , global::esp::box_color1 , cf );
                            ImGui::SameLine( ); ImGui::Text( "box hid" );
                            ImGui::ColorEdit4( "##boxfill" , global::esp::box_fill_color , cf );
                            ImGui::SameLine( ); ImGui::Text( "box fill" );

                            ImGui::ColorEdit4( "##hpcol" , global::esp::health_color , cf );
                            ImGui::SameLine( ); ImGui::Text( "health" );
                            ImGui::ColorEdit4( "##hpcolend" , global::esp::health_color_end , cf );
                            ImGui::SameLine( ); ImGui::Text( "health end" );
                            ImGui::ColorEdit4( "##armorcol" , global::esp::armor_color , cf );
                            ImGui::SameLine( ); ImGui::Text( "armor" );

                            ImGui::ColorEdit4( "##skelcol" , global::esp::skel_color , cf );
                            ImGui::SameLine( ); ImGui::Text( "skel vis" );
                            ImGui::ColorEdit4( "##skelcol1" , global::esp::skel_color1 , cf );
                            ImGui::SameLine( ); ImGui::Text( "skel hid" );

                            ImGui::ColorEdit4( "##namecol" , global::esp::name_color , cf );
                            ImGui::SameLine( ); ImGui::Text( "name vis" );
                            ImGui::ColorEdit4( "##namecol1" , global::esp::name_color1 , cf );
                            ImGui::SameLine( ); ImGui::Text( "name hid" );

                            ImGui::ColorEdit4( "##distcol" , global::esp::distance_color , cf );
                            ImGui::SameLine( ); ImGui::Text( "distance" );
                            ImGui::ColorEdit4( "##weapcol" , global::esp::weapon_color , cf );
                            ImGui::SameLine( ); ImGui::Text( "weapon" );

                            ImGui::ColorEdit4( "##oofcol" , global::esp::oof_color , cf );
                            ImGui::SameLine( ); ImGui::Text( "oof vis" );
                            ImGui::ColorEdit4( "##oofcol1" , global::esp::oof_color1 , cf );
                            ImGui::SameLine( ); ImGui::Text( "oof hid" );

                            ImGui::EndChild( );
                        }
                        ImGui::Columns( 1 );
                    }

                    
                    else if ( selected_tab == 2 )
                    {
                        ImGui::Columns( 2 , "##wpn_cols" , false );
                        {
                            ImGui::BeginChild( "##wpn_left" , ImVec2( 0 , 0 ) , true );

                            ImGui::Checkbox( "no recoil" , &global::weapon::no_recoil );
                            ImGui::Checkbox( "no spread" , &global::weapon::no_spread );
                            ImGui::Checkbox( "automatic" , &global::weapon::automatic );

                            ImGui::Spacing( );
                            ImGui::PushItemWidth( -FLT_MIN );

                            ImGui::SliderFloat( "##ryx" , &global::weapon::recoil_x , -1.f , 1.f , "%.2f" );
                            ImGui::SameLine( ); ImGui::Text( "recoil X scale" );

                            ImGui::SliderFloat( "##ryy" , &global::weapon::recoil_y , -1.f , 1.f , "%.2f" );
                            ImGui::SameLine( ); ImGui::Text( "recoil Y scale" );

                            ImGui::PopItemWidth( );
                            ImGui::EndChild( );
                        }
                        ImGui::NextColumn( );
                        {
                            ImGui::BeginChild( "##wpn_right" , ImVec2( 0 , 0 ) , true );
                            ImGui::EndChild( );
                        }
                        ImGui::Columns( 1 );
                    }

                    
                    else if ( selected_tab == 3 )
                    {
                        ImGui::Columns( 2 , "##misc_cols" , false );
                        {
                            ImGui::BeginChild( "##misc_left" , ImVec2( 0 , 0 ) , true );

                            ImGui::Checkbox( "spiderman"   , &global::misc::spiderman  );
                            ImGui::Checkbox( "no fall dmg" , &global::misc::no_fall    );
                            ImGui::Checkbox( "fov changer" , &global::misc::fov_changer );

                            if ( global::misc::fov_changer )
                            {
                                ImGui::PushItemWidth( -FLT_MIN );
                                ImGui::SliderFloat( "##fovamt" , &global::misc::fov_amount , 60.f , 120.f , "%.0f" );
                                ImGui::SameLine( ); ImGui::Text( "fov" );
                                ImGui::PopItemWidth( );
                            }

                            ImGui::EndChild( );
                        }
                        ImGui::NextColumn( );
                        {
                            ImGui::BeginChild( "##misc_right" , ImVec2( 0 , 0 ) , true );
                            ImGui::EndChild( );
                        }
                        ImGui::Columns( 1 );
                    }

                    ImGui::EndChild( ); 
                }
                ImGui::End( );
            }

            if ( allow_draw )
                esp::get( );

            ImGui::Render( );
            const float cc [ 4 ] = { 0.f, 0.f, 0.f, 0.f };
            d3d_device_ctx->OMSetRenderTargets( 1 , &d3d_render_target , nullptr );
            d3d_device_ctx->ClearRenderTargetView( d3d_render_target , cc );
            ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );
        }
    };

} 

inline overlay::c_overlay* g_overlay = new overlay::c_overlay( );
