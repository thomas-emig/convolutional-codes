# Experiments with convolutional codes

This software is intended to demonstrate different decoding techniques for convolutional codes. The encoder and decoder implementations contained in this repository are very flexible but at the same time not very efficient.

Instead of the correct metrics, the decoder in these examples use metrics which were found to produce good results. For soft-decision decoding, metrics based on the quadratic distance to the expected symbol are used.

## Build:

- `mkir build && cd build`
- `cmake ..`
- `make`

## Changing the simulation parameter

To use different demapper (e.g. soft vs. hard demapper) and decoder (viterbi or stack decoder) implementations, edit the `CMakeLists.txt` file and comment/uncomment the respective components.

The build produces three binaries, one for hard decision simulation on a binary symmetric channel (`binary-symmetric-simulation`), one for soft or hard decision simulation on the AWGN channel (`awgn-simulation`) and one for uncoded transmission on the AWGN channel (`uncoded-simulation`).

Code parameter can be added/edited in `common/codebook.c` and symbol mapping in `common/constellations.c`.

## Further Information

- Slides of the presentation to this project can be found at [https://thomas-emig.de/data/faltungscodes-kurz.pdf] and [https://thomas-emig.de/data/faltungscodes-lang.pdf].
- A very good source of information is Phil Karn's Website [http://www.ka9q.net].
