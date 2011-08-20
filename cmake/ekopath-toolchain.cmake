
# Look for wine compilers
find_program(Ekopath pathcc)
mark_as_advanced(Ekopath)
find_program(EkopathXX pathCC)
mark_as_advanced(EkopathXX)

if((NOT Ekopath) OR (NOT EkopathXX))
	message(FATAL_ERROR "ekopath not found (found: c compiler \"${Ekopath}\", c++ compiler \"${EkopathXX}\")")
endif()

# which compilers to use for C and C++
set(CMAKE_C_COMPILER "${Ekopath}")
set(CMAKE_CXX_COMPILER "${EkopathXX}")
