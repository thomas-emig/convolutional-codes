cmake_minimum_required(VERSION 3.20)

project( convolutional-codes )

set (CMAKE_C_FLAGS      "${CMAKE_C_FLAGS} -O3 -Wall -Wextra -Werror")

# build with asan
#set (CMAKE_C_FLAGS      "${CMAKE_C_FLAGS} -O3 -Wall -Wextra -Werror -fsanitize=address")
#set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

include_directories(
	common/include/
)

add_executable(
	hard-decision-simulation

	hard-decision/main.c
	common/codebook.c
	common/encoder.c
	hard-decision/viterbi-decoder.c
#	hard-decision/stack-decoder.c
#	hard-decision/fano-decoder.c
)

target_include_directories(
	hard-decision-simulation PRIVATE

	hard-decision/include/
)

add_executable(
	soft-decision-simulation

	soft-decision/main.c
	common/codebook.c
	common/encoder.c
	common/constellations.c
	common/mapper.c
	common/demapper.c
#	common/hard-demapper.c
	common/gaussian.c
	soft-decision/viterbi-decoder.c
#	soft-decision/stack-decoder.c
)

target_include_directories(
	soft-decision-simulation PRIVATE

	soft-decision/include/
)

target_link_libraries(
	soft-decision-simulation

	m
)

add_executable(
	uncoded-simulation

	uncoded/main.c
	common/codebook.c
	common/constellations.c
	common/mapper.c
	common/demapper.c
	common/gaussian.c
)

target_link_libraries(
	uncoded-simulation

	m
)
