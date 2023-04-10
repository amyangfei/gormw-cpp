add_library(
  gor
  OBJECT
  gor.cpp
)

set(ALL_OBJECT_FILES
  ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:gor>
  PARENT_SCOPE) 