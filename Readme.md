# Experiments with convolutional codes

## Build:

- `mkir build && cd build`
- `cmake ..`
- `make`

## Changing the simulation parameter

To use different demapper (e.g. soft vs. hard demapper) and decoder (viterbi or stack decoder) implementations, edit the `CMakeLists.txt` file and comment/uncomment the respective components.

The build produces three binaries, one for hard decision simulation on a binary symmetric channel (`hard-decision-simulation`), one for soft or hard decision simulation on the AWGN channel (`soft-decision-simulation`) and one for uncoded transmission on the AWGN channel (`uncoded-simulation`).

Code parameter can be added/edited in `common/codebook.c` and symbol mapping in `common/constellations.c`.
