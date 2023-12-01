cmake_minimum_required(VERSION 3.5.0)

project(db VERSION 1.0.0 LANGUAGES C)

add_compile_options(-Wall)

if (USE_WEXTRA)
	add_compile_options(-Wextra -pedantic -Wno-unused-parameter)
endif()
if (USE_WERROR)
	add_compile_options(-Werror)
endif()

find_package(PkgConfig REQUIRED)

pkg_check_modules(MYSQL REQUIRED libmariadb)

include_directories(db ${MYSQL_INCLUDE_DIRS})

add_executable(db
	db.c
	)
target_link_libraries(db ${MYSQL_LIBRARIES})
