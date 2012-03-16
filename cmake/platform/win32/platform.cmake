################################################################################
# set some windows specific variables
################################################################################

# some boost options
add_definitions(-DBOOST_FILESYSTEM_VERSION=2 -D_ITERATOR_DEBUG_LEVEL=0)

# some wxWidget options
macro(use_unicode_here)
  add_definitions(-D_UNICODE)
  add_definitions(-DwxUSE_UNICODE)
endmacro()

if(DEBUG)
    add_definitions(-DDEBUG -D_DEBUG)
else()
    add_definitions(-DNDEBUG -D_NDEBUG)
endif()

# from official repo, do we need them?
# /ignore:4006
# /ignore:4099
# /ignore:4221
#add_definitions()
