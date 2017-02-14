# Matlab visualizer

## Usage

The _cometa_emg_yarp_ module dumps raw data files in the directory from where it is launched. To plot realtime raw data, you can use our Matlab visualizer as follows

    realtime_plot <path_to_folder_containing_rawEmgXXX_files>

This will look for the newest _rawEmg\__ file in the provided folder, and plot the data that are written into it. Please note that you should launch the visualizer __after__ the acquisition module, so that the correct file is found.


## Disclaimer

Files ``parseArgs.m`` and ``subaxis.m`` have been downloaded from:

https://es.mathworks.com/matlabcentral/fileexchange/3696-subaxis-subplot

The license for those files is in license.txt.
