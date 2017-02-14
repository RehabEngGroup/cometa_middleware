# Cometa EMG on YARP&ROS

Software to read [Cometa](http://www.cometasystems.com/) electromyography (EMG) signals within [ROS](http://www.ros.org/) or [YARP](http://yarp.it/).
Currently, only the YARP-based version is available, as the USB drivers and C API for the [Cometa Wave Wireless EMG](http://www.cometasystems.com/products/wave-wire%C2%ADless-emg)
system are only available for Windows. This YARP module however is compatible with ROS [1][2] and ROS nodes can receive and consume its data.


## Usage

Basic documentation for the _cometa_emg_yarp_ module can be found in its [xml description](yarp_version/cometa_emg_yarp.xml).

The module retrieves from the ROS parameter server an ``emgNames`` _dict_ parameter. The keys in this dictionary (or map) represent the muscle names that are being measured.
For each muscle, the ``chan`` property is the number of the Cometa Wave probe that is placed on that muscle, and the ``maxEnv`` property is the value by which the computed envelope is divided to output normalized data.

If the _emgNames_ parameter is not correctly retrieved, all EMG channels are collected, and the name _Emg Chan XXX_ is associated to each channel (_XXX_ is 0 for the first channel).
Otherwise, only the channels listed as _chan_ for a muscle are activated.
The _maxEnv_ property is optional; if it is missing, data for that muscle will not be normalized.

    TODO: ensure that the chan property is found for each muscle.

In order to reduce network load, the module processes raw data to compute the linear envelope, which can be output at a reduced rate without significant data loss.
Data are first bandpass filtered with a 2nd order Butterworth filter (20-300 Hz), then rectified and low-pass filtered at 8 Hz, again with a 2nd order Butterworth filter.

At the moment, the module publishes an envelope message every 10 samples read from the Cometa (i.e., at 200 Hz, since data acquisition runs at 2000Hz).
Since data are read as USB packets, the frequency at which the messages are written depends on the packet size (see __Dependencies__ section);
the timestamp in each message however contains the correct acquisition time.

### Extras

At the moment, the module also dumps raw data to file, for debugging purposes.
The file is created in the directory from where the executable is launched, and it is named _rawEmg_DATE_AND_TIME.txt_.
Columns correspond to EMG channels, and are ordered by channel number (this is different from the content of the ROS message, where the channels are listed alphabetically).
Please note that only channels activated through the _emgNames_ ROS parameter are recorded and dumped.

    TODO: provide CMake option to disable raw data dumping

Raw data files can also be used to visualize raw data online through the [Matlab visualizer ](utils/matlabVisualizer/realtime_plot.m).
This is quite useful to verify that data acquisition is going smoothly and there are no artefacts (electrodes detached or spurious hits).

## Dependencies

* Cometa drivers and WaveAPI library, provided by Cometa Systems. A custom version of the drivers was created for us, which outputs smaller USB packets, at higher speed (20 samples per packet instead of 200).
* [Filter](https://github.com/RealTimeBiomechanics/Filter) library, for online processing
* [YARP](http://yarp.it/) middleware. Tested with Yarp 2.3.63 for VS10, 32 bit.

## Acknowledgements

We thank Matteo Dellacorna and all the Cometa technical staff for providing custom drivers and support.

#### References

[1] Ceseracciu et al., _A flexible architecture to enhance wearable robots: Integration of EMG-informed models_, 2015 IEEE/RSJ International Conference on Intelligent Robots and Systems (IROS), pp. 4368-4374, 2015.

[2] Natale et al., _The iCub Software Architecture: Evolution and Lessons Learned_, Frontiers in Robotics and AI 3, p. 24, 2016.
