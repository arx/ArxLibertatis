
function(print_configuration TITLE)
	
	set(str "")
	
	set(print_first 0)
	
	set(mode 0)
	
	foreach(arg IN LISTS ARGN)
		
		if(arg STREQUAL "FIRST")
			set(print_first 1)
		else()
			
			if(mode EQUAL 0)
				
				if(${arg})
					set(mode 1)
				else()
					set(mode 2)
				endif()
				
			else()
				
				if(mode EQUAL 1 AND NOT arg STREQUAL "")
					
					if(str STREQUAL "")
						set(str "${arg}")
					else()
						set(str "${str}, ${arg}")
					endif()
					
					if(print_first)
						break()
					endif()
					
				endif()
				
				set(mode 0)
				
			endif()
			
		endif()
		
	endforeach()
	
	if(str STREQUAL "")
		set(str "(none)")
	endif()
	
	message(" - ${TITLE}: ${str}")
	
endfunction()
