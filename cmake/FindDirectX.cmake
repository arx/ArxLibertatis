
# Try to find DirectX libraries and include paths.
# Once done this will define
#
# DIRECTX_FOUND
# DIRECTX_INCLUDE_DIR
# DIRECTX_LIBRARIES
#
# Adapted from http://code.google.com/p/gamekit/source/browse/trunk/CMake/Packages/FindDirectX.cmake

include(CompileCheck)

if(WIN32)
	
	# include dir
	find_path(DIRECTX_INCLUDE_DIR NAMES d3d9.h)
	
	find_library(DIRECTX_D3D9_LIBRARY NAMES d3d9)
	find_library(DIRECTX_D3DX9_LIBRARY NAMES d3dx9)
	find_library(DIRECTX_DXERR9_LIBRARY NAMES dxerr)
	find_library(DIRECTX_DXGUID_LIBRARY NAMES dxguid)
	find_library(DIRECTX_DINPUT8_LIBRARY NAMES dinput8)
	find_library(DIRECTX_XINPUT_LIBRARY NAMES xinput)
	find_library(DIRECTX_DXGI_LIBRARY NAMES dxgi)
	find_library(DIRECTX_D3DCOMPILER_LIBRARY NAMES d3dcompiler)
	
	set(DIRECTX_LIBRARY 
		${DIRECTX_D3D9_LIBRARY} 
		${DIRECTX_D3DX9_LIBRARY}
		${DIRECTX_DXERR9_LIBRARY}
		${DIRECTX_DXGUID_LIBRARY}
		${DIRECTX_DINPUT8_LIBRARY}
		${DIRECTX_XINPUT_LIBRARY}
		${DIRECTX_DXGI_LIBRARY}
		${DIRECTX_D3DCOMPILER_LIBRARY}
	)
	
	# look for D3D10.1 components
	if (DIRECTX_INCLUDE_DIR)
		find_path(DIRECTX_D3D10_INCLUDE_DIR NAMES d3d10_1shader.h NO_DEFAULT_PATH)
		get_filename_component(DIRECTX_LIBRARY_DIR "${DIRECTX_LIBRARY}" PATH)
		
		find_library(DIRECTX_D3D10_LIBRARY NAMES d3d10 NO_DEFAULT_PATH)
		find_library(DIRECTX_D3DX10_LIBRARY NAMES d3dx10 NO_DEFAULT_PATH)
		
		if (DIRECTX_D3D10_INCLUDE_DIR AND DIRECTX_D3D10_LIBRARY AND DIRECTX_D3DX10_LIBRARY)
			set(DIRECTX_D3D10_FOUND TRUE)
			set(DIRECTX_D3D10_INCLUDE_DIRS ${DIRECTX_D3D10_INCLUDE_DIR})
	  		set(DIRECTX_D3D10_LIBRARIES ${DIRECTX_D3D10_LIBRARY} ${DIRECTX_D3DX10_LIBRARY}) 
		endif ()
	endif ()
	
	# look for D3D11 components
	if (DIRECTX_INCLUDE_DIR)
		find_path(DIRECTX_D3D11_INCLUDE_DIR NAMES D3D11Shader.h NO_DEFAULT_PATH)
		get_filename_component(DIRECTX_LIBRARY_DIR "${DIRECTX_LIBRARY}" PATH)
		
		find_library(DIRECTX_D3D11_LIBRARY NAMES d3d11 d3d11_beta HINTS NO_DEFAULT_PATH)
		find_library(DIRECTX_D3DX11_LIBRARY NAMES d3dx11 NO_DEFAULT_PATH)
		find_library(DIRECTX_D3DX11COMPILER_LIBRARY NAMES d3dcompiler NO_DEFAULT_PATH)
		find_library(DIRECTX_DXGI_LIBRARY NAMES dxgi NO_DEFAULT_PATH)
		
		if (DIRECTX_D3D11_INCLUDE_DIR AND DIRECTX_D3D11_LIBRARY AND DIRECTX_D3DX11_LIBRARY)
			set(DIRECTX_D3D11_FOUND TRUE)
			set(DIRECTX_D3D11_INCLUDE_DIRS ${DIRECTX_D3D11_INCLUDE_DIR})
			set(DIRECTX_D3D11_LIBRARIES ${DIRECTX_D3D11_LIBRARY} ${DIRECTX_D3DX11_LIBRARY} ${DIRECTX_D3DX11COMPILER_LIBRARY} ${DIRECTX_DXGI_LIBRARY})
		endif ()
	endif ()
	
	# handle the QUIETLY and REQUIRED arguments and set DIRECTX_FOUND to TRUE if 
	# all listed variables are TRUE
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(DirectX DEFAULT_MSG DIRECTX_LIBRARY DIRECTX_INCLUDE_DIR)
	
elseif(WINE)
	
	function(__check_wine_dx_library VARNAME LIB)
		
		try_link_library(${VARNAME} "${LIB}" ERR)
		
		if(CHECK_${VARNAME}_LINK)
			set(${VARNAME} "${LIB}" PARENT_SCOPE)
		else()
			message(STATUS "Missing DX library ${LIB}.")
			set(${VARNAME} "${VARNAME}-NOTFOUND" PARENT_SCOPE)
		endif()
		
	endfunction(__check_wine_dx_library)
	
	# wineg++ will handle this automatically
	set(DIRECTX_INCLUDE_DIR "")
	
	__check_wine_dx_library(DIRECTX_D3D9_LIBRARY d3d9)
	__check_wine_dx_library(DIRECTX_D3DX9_LIBRARY d3dx9)
	__check_wine_dx_library(DIRECTX_DXERR9_LIBRARY dxerr9)
	__check_wine_dx_library(DIRECTX_DXGUID_LIBRARY dxguid)
	__check_wine_dx_library(DIRECTX_DINPUT8_LIBRARY dinput8)
	__check_wine_dx_library(DIRECTX_XINPUT_LIBRARY xinput)
	__check_wine_dx_library(DIRECTX_DXGI_LIBRARY dxgi)
	__check_wine_dx_library(DIRECTX_D3DCOMPILER_LIBRARY d3dcompiler)
	
	set(DIRECTX_LIBRARY 
		${DIRECTX_D3D9_LIBRARY} 
		${DIRECTX_D3DX9_LIBRARY}
		${DIRECTX_DXERR9_LIBRARY}
		${DIRECTX_DXGUID_LIBRARY}
		${DIRECTX_DINPUT8_LIBRARY}
		${DIRECTX_DXGI_LIBRARY}
		${DIRECTX_D3DCOMPILER_LIBRARY}
	)
	
	# handle the QUIETLY and REQUIRED arguments and set DIRECTX_FOUND to TRUE if 
	# all listed variables are TRUE
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(DirectX DEFAULT_MSG DIRECTX_LIBRARY)
	
endif()

if(DIRECTX_FOUND)
	set(DIRECTX_LIBRARIES ${DIRECTX_LIBRARY})
endif(DIRECTX_FOUND)
