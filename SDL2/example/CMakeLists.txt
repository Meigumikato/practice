cmake_minimum_required(VERSION 3.20)

project(example)

function(example)
  add_executable(${ARGV0})
  target_sources(${ARGV0} PRIVATE ${ARGV1})
  target_compile_features(${ARGV0} PRIVATE cxx_std_17)
  target_compile_options(${ARGV0} PRIVATE -fsanitize=address)
  target_link_options(${ARGV0} PRIVATE -fsanitize=address)
  target_link_libraries(
    ${ARGV0} PRIVATE SDL2::SDL2 SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf
                     SDL2_mixer::SDL2_mixer)
endfunction(example)

example(geometry geometry.cpp)
example(clip_render clip_render.cpp)
example(modulate modulate.cpp)
example(alpha_blend alpha_blend.cpp)
example(animation animation.cpp)
example(rotate_and_flip rotate_and_flip.cpp)
example(ttf_render ttf_render.cpp)
example(mouse_event mouse_event.cpp)
example(key_state key_states.cpp)
example(sound sound.cpp)
example(time timing.cpp)
example(calculate_fps calculate_fps.cpp)
