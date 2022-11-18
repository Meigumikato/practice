cmake_minimum_required(VERSION 3.20)

project(tutorials)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

function(sdl_tutorials)

  set(SRC)
  add_executable(${ARGV0})
  aux_source_directory(${ARGV1} SRC)
  target_sources(${ARGV0} PRIVATE ${SRC})
  target_compile_features(${ARGV0} PRIVATE cxx_std_17)
  target_link_libraries(
    ${ARGV0} PRIVATE SDL2::SDL2 SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image
                     SDL2_mixer::SDL2_mixer)
endfunction(sdl_tutorials)

sdl_tutorials(hello 01_hello_SDL)
sdl_tutorials(getting_an_image_on_the_screen 02_getting_an_image_on_the_screen)
sdl_tutorials(event_driven_programming 03_event_driven_programming)
sdl_tutorials(key_presses 04_key_presses)
sdl_tutorials(optimized_surface_loading_and_soft_stretching
              05_optimized_surface_loading_and_soft_stretching)
sdl_tutorials(extension_libraries_and_loading_other_image_formats
              06_extension_libraries_and_loading_other_image_formats)
sdl_tutorials(texture_loading_and_rendering 07_texture_loading_and_rendering)
sdl_tutorials(geometry_rendering 08_geometry_rendering)
sdl_tutorials(the_viewport 09_the_viewport)
sdl_tutorials(color_keying 10_color_keying)
sdl_tutorials(clip_rendering_and_sprite_sheets
              11_clip_rendering_and_sprite_sheets)

sdl_tutorials(color_modulation 12_color_modulation)
sdl_tutorials(alpha_blending 13_alpha_blending)
sdl_tutorials(animated_sprites_and_vsync 14_animated_sprites_and_vsync)

sdl_tutorials(rotation_and_flipping 15_rotation_and_flipping)
sdl_tutorials(true_type_fonts 16_true_type_fonts)
sdl_tutorials(mouse_events 17_mouse_events)
sdl_tutorials(key_states 18_key_states)
sdl_tutorials(gamepads_and_joysticks 19_gamepads_and_joysticks) # game
                                                                # controller
sdl_tutorials(force_feedback 20_force_feedback) # game controller
sdl_tutorials(sound_effects_and_music 21_sound_effects_and_music)
sdl_tutorials(timing 22_timing)
sdl_tutorials(advance_timer 23_advanced_timers)
sdl_tutorials(calculating_frame_rate 24_calculating_frame_rate)
sdl_tutorials(capping_frame_rate 25_capping_frame_rate)

add_subdirectory(example)
