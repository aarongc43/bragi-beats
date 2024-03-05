# Drag-and-Drop Music File Handling

- Monitor Drag-and-Drop Events
  `IsFileDropped()`
  `GetDroppedFiles()`
- Load and Play the Dropped Music Files
  `LoadMusicStream()`
  `PlayMusicStream()`

# Install FFTW 

`brew install fftw`

include `fftw3` into build.sh

`-I/opt/homebrew/Cellar/fftw/3.3.10_1/include -L/opt/homebrew/Cellar/fftw/3.3.10_1/lib -lfftw3`

# How to analyze music track

- preload entire wave
- preprocess it 
- as music is playing, get time 
- by the time we know where the preprocessed data is
- we will know where it is located

DFT is more general
- DFT O(N^2)
- FFT O(NlogN)
