cmake_minimum_required(VERSION 3.13.1)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(PSI3441)

# Headers da NXP SDK local: MKL25Z4.h, pwm_z42.h
include_directories(../include)

# Fontes em src/ (main.c + pwm_z42.c copiado de 5/)
FILE(GLOB app_sources ../src/*.c*)
target_sources(app PRIVATE ${app_sources})
