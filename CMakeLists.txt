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
	binary-symmetric-simulation

	binary-symmetric-channel/main.c
	common/codebook.c
	common/encoder.c
	binary-symmetric-channel/viterbi-decoder.c
#	binary-symmetric-channel/stack-decoder.c
#	binary-symmetric-channel/fano-decoder.c
)

target_include_directories(
	binary-symmetric-simulation PRIVATE

	binary-symmetric-channel/include/
)

add_executable(
	awgn-simulation

	AWGN-channel/main.c
	common/codebook.c
	common/encoder.c
	common/constellations.c
	common/mapper.c
	common/demapper.c
#	common/hard-demapper.c
	common/gaussian.c
	AWGN-channel/viterbi-decoder.c
#	AWGN-channel/stack-decoder.c
#	AWGN-channel/fano-decoder.c
)

target_include_directories(
	awgn-simulation PRIVATE

	AWGN-channel/include/
)

target_link_libraries(
	awgn-simulation

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
