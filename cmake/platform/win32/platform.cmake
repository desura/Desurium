################################################################################
# set some windows specific variables
################################################################################

# some boost options
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
add_definitions(-DBOOST_FILESYSTEM_VERSION=2 -D_ITERATOR_DEBUG_LEVEL=0)

# some wxWidget options
macro(use_unicode_here)
  add_definitions(-D_UNICODE)
  add_definitions(-DwxUSE_UNICODE)
endmacro()

# from official repo, do we need them?
add_definitions(/ignore:4006
                /ignore:4099
                /ignore:4221)
