# Application Requirements

## Requirements

These are requirements that need to be accessible to the user.

### Level 1

- Drag and Drop music.
- Music will be loaded into a visual queue
- Play back controls
- Visualization Picker

### Level 2 User

- Login Feature for different Users
- SQL Database for User access
- SQL database will store library, visualizers.
- Local Server
- Saved Music and Library
- Saved Configurations
- Library Management
- More Visualizations
- Visualization Customizations


## Modules

- **Audio Loading** - What audio are we going to be loading, FLAC, MP3, WAV etc.

```C
loadAudioFile(filename)
playAudio()
pauseAudio()
stopAudio()
seekAudio(timePosition)
getAudioVolume()
setAudioVolume(volume)
```

- **FFT Analysis** - Custom FFT library, handle frequency binning, buffers for
  frequency spectrum data for visualizations.

```C
initFFT(fftSize) // initialize FFT module will be called after drag and drop
performFFT(inputAudioData, outputSpectrum) // calls FFT functions and stores
    //frequency amplitudes in an output buffer
```

- **Visualization Core** - Raylib rendering loop. This will coordinate which visual elements are being implemented

```C
initBarVisualizer() // data structure for the bar visualizer
updateBarVisualizer(fftSpectrum) // takes the FFT output and maps amplitudes to
    //bar heights, positions, colors, etc.
renderBarVisualizer() // Executes the drawing of bars using raylib functions
mapValue() // generic function to scale values from one range to another
```

- **Visualization Types** - What visualizers are we going to be using?

- **User Input** - User input to Play/Pause, order Queue, volume, skip,
  visualization adjustments like intensity, colors etc.

```C
createButton(buttonName, position, size)
isButtonClicked(buttonName)
createSlider(sliderName, position, size, minVal, maxVal)
getSliderValue(slidername)

```

- **Configuration and Settings** - Load user's last used visualizers types based on
  each account created, save library, load metadata and modify that metadata.

# Iterations

## Iteration 1

- Audio Loading
- FFT Analysis
- Bar Chart Visualizer

# Testing
