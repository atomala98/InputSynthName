# InputSynthName

Granular synthesizer, created in JUCE framework.

![image](https://user-images.githubusercontent.com/57626423/171354304-c2f55302-f394-4a7f-8113-31ecb218e772.png)

Parameters:
- Grain start - number of sample where first grain in grain cloud is started. Depends on the length of an input sample.
- Grain length - length of every grain in the cloud in milliseconds
- Grain cloud size - number of grains in the grain cloud
- Grain randomization - grains starting from grain 2 in a cloud have randomised start, and staring sample is calculated as start of first grain + randomized value from range (-grain randomisation, +grain randomisation)
- Grain max/min gain - randomizes volume of each grain
- Output gain - pretty self-explanatory
- Overdrive - additional overdrive effect

 [InputNameHere.zip](https://github.com/atomala98/InputSynthName/files/8812196/InputNameHere.zip) <- link to compiled VST
