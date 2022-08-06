-- premake5.lua
workspace "OpenField"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "OpenField"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "WalnutExternal.lua"
include "OpenField"