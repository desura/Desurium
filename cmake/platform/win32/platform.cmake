################################################################################
# set some windows specific variables
################################################################################

# some boost options
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_STATIC_RUNTIME     ON)

add_definitions(/ignore:4006
                /ignore:4099
                /ignore:4221)
