# Stereogram

Rendering of ['random dot stereograms'](https://en.wikipedia.org/wiki/Autostereogram) for pseudo 3D view.
This program provides routines for generating autostereogram.
It's just for fun and educational purpose. Feel free to modify, improve, and use it :)

Try it out: you have to use the original size of 512px * 512px. Just, squint your eyes and try to focus behind the image:
 
![Autostereogram|512](https://raw.githubusercontent.com/Bondoki/Stereogram/refs/heads/assets/output.png)  

Here's the depth map of the incorporated image:  

![DepthMap|300](https://raw.githubusercontent.com/Bondoki/Stereogram/refs/heads/assets/DepthMap.png)
 
## Installation

Variant A) Just do for standard compilation (tested with gcc 13.3.0):

````sh
    # generates the application
    g++ -o Stereogram Stereogram.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17 -Wno-narrowing

    # run the application
    ./Stereogram
````

Variant B) Visit the PGETinker website with the [running stereogram](https://pgetinker.com/s/1RbrBSpufLN).  

Variant C) Use your favorite browser and open the [Stereogram.html](https://raw.githubusercontent.com/Bondoki/Stereogram/refs/heads/main/Stereogram.html).


## Usage

Simply use the mouse to pan the image.
There's no zoom because the autostereogram relies on "correct pixelation"
and zooming would result in "blurred pixels".  
  
Use the left mouse button to pan the image.  
  
Use key **Q** for background image.  
Use key **W** for depth map image.  
Use key **A** for stereogram with shift/slice algorithm.  
Use key **S** for stereogram with algorithm by [Thimbleby,Inglis,Witten](https://hdl.handle.net/10289/9920).  
Use **SPACE** to toggle between both stereograms.  
Use **ENTER** to save view to *'output.png'*.  
  
Use key **ESC** to quit the application.  


## References, License, Credit, Acknowledgment
The program acknowledge the libraries and refer to the appropriate licenses
* [olcPixelGameEngine](https://github.com/OneLoneCoder/olcPixelGameEngine) by [Javidx9](https://github.com/OneLoneCoder)
* [olcPGEX_TransformedView](https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/extensions/olcPGEX_TransformedView.h) by [Javidx9](https://github.com/OneLoneCoder)
* Thimbleby, Harold W., Stuart Inglis, and Ian H. Witten. "[Displaying 3D images: Algorithms for single-image random-dot stereograms.](https://hdl.handle.net/10289/9920)"; Computer 27.10 (1994): 38-48
* Big Thanks to the [PGEtinker community](https://pgetinker.com/) giving me the possibility to share the application without installation ✨  
* Other pieces of codes are released under [The Unlicense](https://github.com/Bondoki/Stereogram/blob/main/LICENSE) into public domain for free usage.
