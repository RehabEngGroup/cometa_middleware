// Author: Elena Ceseracciu, November 2014

#include <stdio.h>

#include <time.h>

#include "WaveAPI.h" // To use the WaveAPI library
#include <yarp/os/Network.h>
#include <yarp/os/Time.h>
#include <yarp/os/Node.h>
#include <yarp/os/Publisher.h>
#include <yarp/os/RFModule.h>

#include "ceinms_msgs_EmgData.h"
#include <vosl/Filter/Filter.h>
#include <vosl/Filter/Designer.h>


const unsigned int COMETA_SAMPLING_RATE = 2000;

struct EnvelopeSample
{
    double time;
    std::vector<double> data;

    EnvelopeSample() {};

    EnvelopeSample(size_t size)
    {
        data.resize(size);
    }
};

class CometaReader
{
    //taken from WaveAPI example:
    WaveDevice *device;
    int emgChanEnableVect[MAX_EMG_INSTALLED_CHAN];
    int fswChanEnableVect[MAX_FSW_INSTALLED_CHAN];
    int fswThresholdVect[MAX_FSW_INSTALLED_CHAN * FSW_TRANSDUCERS];
    int fswProtocolType;
    int trgMode;
    int emgInstalledChanNum;
    int fswInstalledChanNum;
    short int * dataBuff;
    unsigned int dataNum;
    int isAcqRunning;
    int startFromTrg, stopFromTrg;
    unsigned int firstSample, lastSample;
    int errcode;
    int i, t;

    unsigned int emgEnabledChanNum;
    std::vector<int> channelsMap;
    // Bandpass filter for raw EMG signal
    std::vector<VOSL::Filter::Filter<double> > filters;
    // Lowpass filter for rectified EMG signal
    std::vector<VOSL::Filter::Filter<double> > filtersLE;
    // Maximum value (so far) for envelopes, used for normalization
    std::vector<double> maxEnvs;
    size_t sampleCounter;
    bool newEpoch;
    bool initialized;
    bool selfNormalize;

public:
    //only works if called before initialize
    bool setTriggerIn(bool on = true)
    {
        if (initialized)
            return false;
        if (on)
            trgMode = DAQ_TRG_IN_ACTIVE_HI;
        else
            trgMode = DAQ_TRG_OUT_ENABLE;
        return true;
    }

    bool setSelfNormalize(bool on = true)
    {
        if (initialized)
            return false;
        selfNormalize = on;
        return true;
    }

    int initialize(std::vector<int> enabledChans)
    {
        if (device == NULL)
        {
            std::cout << "Cannot access the device!" << std::endl;
            return -1;
        }
        // step 3: configure acquisition parameters
        emgEnabledChanNum = enabledChans.size();
        if (emgEnabledChanNum == 0)
        {
            emgEnabledChanNum = emgInstalledChanNum;
            //enable all the Emg channels (1= enabled, 0= disabled)
            for (i = 0; i < emgInstalledChanNum; i++)
            {
                emgChanEnableVect[i] = 1;
                channelsMap.push_back(i);
            }
        }
        else
        {
            channelsMap.clear();
            for (i = 0; i < emgInstalledChanNum; i++)
            {
                std::vector<int>::iterator foundChan = std::find(enabledChans.begin(), enabledChans.end(), i+1); // ELECTRODE IDs START WITH 1 !
                if (foundChan != enabledChans.end())
                {
                    emgChanEnableVect[i] = 1;
                    channelsMap.push_back(foundChan-enabledChans.begin());
                }
                else
                    emgChanEnableVect[i] = 0;
            }
        }


        if (channelsMap.size() != emgEnabledChanNum)
        {
            std::cout << "Something went wrong in the configuration of the channels mapping" << std::endl;
            return -1;
        }


        //disable all the FootSw channels (1= enabled, 0= disabled) 
        for (i = 0; i < fswInstalledChanNum; i++)
        {
            fswChanEnableVect[i] = 0;
            ////set the FootSw transducers threshold
            //for (t = 0; t<FSW_TRANSDUCERS; t++)
            //    fswThresholdVect[i*FSW_TRANSDUCERS + t] = 20;
        }
        if ((errcode = device->configure(emgChanEnableVect, fswChanEnableVect, fswThresholdVect, fswProtocolType, trgMode)) != 0)
        {
            printf("\nError executing configure() method - Error code= %d\n", errcode);
            DeleteWaveDevice();
            return errcode;
        }

        // step 3b: create online filters

        //high pass 20 hz - Butterworth 2nd order, sampling rate 2000 Hz
        std::vector<std::complex<double> > bCoeffHP;
        bCoeffHP.reserve(3);
        bCoeffHP.push_back(0.9565);
        bCoeffHP.push_back(-1.9131);
        bCoeffHP.push_back(0.9565);

        std::vector<std::complex<double> > aCoeffHP;
        aCoeffHP.reserve(3);
        aCoeffHP.push_back(1.0000);
        aCoeffHP.push_back(-1.9112);
        aCoeffHP.push_back(0.9150);


        VOSL::Filter::Polynomial<double> bHP(bCoeffHP);
        VOSL::Filter::Polynomial<double> aHP(aCoeffHP);


        VOSL::Filter::TransferFunction<double> emgFilterTF =
            VOSL::Filter::butter<double>(2, 300, 2000) * VOSL::Filter::TransferFunction<double>(bHP, aHP, 2000);

        for (int i = 0; i < emgEnabledChanNum; ++i)
        {
            filters.push_back(VOSL::Filter::Filter<double>(emgFilterTF));
        }

        for (int i = 0; i < emgEnabledChanNum; ++i)
        {
            filtersLE.push_back(VOSL::Filter::Filter<double>(VOSL::Filter::butter<double>(2, 8, 2000)));
        }

        // step 3c: initialize maximum envelope values
        for (int i = 0; i < emgEnabledChanNum; ++i)
        {
            if (selfNormalize)
                maxEnvs.push_back(-std::numeric_limits<double>::infinity());
            else
                maxEnvs.push_back(1.0);
        }


        // step 4: activate the device hardware according to the settings defined by configure method
        if ((errcode = device->activate()) != 0)
        {
            printf("\nError executing activate() method - Error code= %d\n", errcode);
            DeleteWaveDevice();
            return errcode;
        }

        // step 5: start data acquisition
        if ((errcode = device->run()) != 0)
        {
            printf("\nError executing run() method - Error code= %d\n", errcode);
            DeleteWaveDevice();
            return errcode;
        }
        initialized = true;
        return 0;
    };

    CometaReader(){
        //initialize variables
        fswProtocolType = FSW_PROT_QUARTER_FOOT;
        trgMode = DAQ_TRG_OUT_ENABLE;
        emgInstalledChanNum = -1;
        fswInstalledChanNum = -1;
        newEpoch = true;
        initialized = false;
        sampleCounter = 0;
        selfNormalize = true;
        device = NULL;

        //step 1: create Wave device
        device = CreateWaveDevice();
        if (device == NULL)
        {
            printf("\nError creating device");
        }
        else
        {
            // step 2: get the Emg and FootSw installed channels number
            if ((errcode = device->getInstalledChan(&emgInstalledChanNum, &fswInstalledChanNum)) != 0)
            {
                printf("\nError executing getInstalledChan() method - Error code= %d\n", errcode);
                DeleteWaveDevice();
                device = NULL;
            }
        }
    };


    ~CometaReader(){
        // step 7: stop the data acquisition process
        errcode = device->stop();
        //            if((errcode= device->stop()) != 0)
        //            {
        //                printf("\nError executing stop() method - Error code= %d\n", errcode);
        //            }

        // step 8: destroy the device
        DeleteWaveDevice();
    };

    void reset()
    {
        newEpoch = true;
    }

    int getInstalledChanNum()
    {
        return emgInstalledChanNum;
    }

    EnvelopeSample getSample()
    {
        std::vector<double> currentSample(emgEnabledChanNum);
        errcode = device->transferData(&dataBuff, &dataNum, &isAcqRunning, &startFromTrg, &stopFromTrg,
            &firstSample, &lastSample);
        if (errcode == 0)
        {
            if (trgMode != DAQ_TRG_OUT_ENABLE && newEpoch)
            {
                if (startFromTrg == 0)
                    return EnvelopeSample();
                else
                    newEpoch = false;
            }
            size_t offset = firstSample;
            size_t curChan = 0;
            while (offset <= lastSample - emgEnabledChanNum )
            {
                for (curChan = 0; curChan < emgEnabledChanNum; curChan++)
                {
                    double sampleValue = filtersLE.at(curChan).filter(abs(filters.at(curChan).filter(dataBuff[offset + curChan])));
                    if (selfNormalize && sampleValue>maxEnvs.at(curChan))
                        maxEnvs.at(curChan) = sampleValue;
                    currentSample.at(curChan) = sampleValue;
                }
                offset += emgEnabledChanNum;
                sampleCounter++;
            }
            EnvelopeSample normalizedReorderedSample(emgEnabledChanNum);
            normalizedReorderedSample.time = (double)sampleCounter / double(COMETA_SAMPLING_RATE);
            for (size_t i = 0; i < channelsMap.size(); ++i)
                normalizedReorderedSample.data.at(i) = currentSample.at(channelsMap[i]) / maxEnvs.at(channelsMap[i]);
            return normalizedReorderedSample;
        }
        return EnvelopeSample();
    };
};


class CometaReaderModule : public yarp::os::RFModule
{
public:
    CometaReaderModule() : node("/cometaReader"), seq(0) { };

    bool configure(yarp::os::ResourceFinder& rf)
    {
        emgPort.topic("/emgData");

        int installedChans = cometa.getInstalledChanNum();
        std::vector<int> enabledChannels;
        if (installedChans < 0)
            return false;

        //query parameter server
        yarp::os::Bottle query, reply;
        query.addString("getParam");
        query.addString("/cometaReader");
        query.addString("emgNames");
        yarp::os::ContactStyle style;
        style.quiet = false;
        style.timeout = 4;
        style.carrier = "xmlrpc";
        bool ok = yarp::os::Network::write(yarp::os::Contact::byName("/ros"), query, reply, style);
        if (!ok)
        {
            yWarning(" Could not connect to ROS parameter server to retrieve list of emgNames - chans");
            defaultEmgNames(installedChans);
        }
        else
        {
            if (reply.get(0).asInt() != 1)
            {
                yWarning(" ROS parameter server did not successfully provide list of emgNames - chans");
                defaultEmgNames(installedChans);
            }
            else
            {
                emgChannelNames.clear();
                yarp::os::Bottle* names = reply.get(2).asList();
                if (names->get(0).asString() != "dict")
                {
                    yWarning(" Did not receive a dict!");
                    defaultEmgNames(installedChans);
                }
                else
                    for (int bt = 1; bt < names->size(); ++bt)
                    {
                        emgChannelNames.push_back(names->get(bt).asList()->get(0).asString());
                        enabledChannels.push_back(names->get(bt).asList()->get(1).asInt()); //TODO check it is consistent with max num emg channels
                    }
            }
        }

        if (rf.check("trigIn"))
            cometa.setTriggerIn(true);
        if (rf.check("selfNormalize"))
            cometa.setSelfNormalize(rf.find("selfNormalize").asBool());

        return cometa.initialize(enabledChannels) == 0;
    };

    void defaultEmgNames(size_t nChannels)
    {
        emgChannelNames.clear();
        for (size_t n = 0; n < nChannels; ++n)
        {
            std::string chanName = "Emg Chan "  + std::to_string(n);
            emgChannelNames.push_back(chanName);
        }
    }

    bool updateModule(){
        EnvelopeSample sample = cometa.getSample();

        if (sample.data.size() < 0)
            return true;
        ceinms_msgs_EmgData& sampleOnPort = emgPort.prepare();
        sampleOnPort.header.seq = seq++;
        sampleOnPort.header.stamp.sec = sample.time;
        sampleOnPort.header.stamp.nsec = (sample.time - sampleOnPort.header.stamp.sec) * 1000000000;
        sampleOnPort.name = emgChannelNames;
        if (emgChannelNames.size() != sample.data.size()) //not sure if this is needed...
        {
            yError("Something went wrong while reading data from the Cometa, I got a different number of signals than expected");
            return false;
        }

        sampleOnPort.envelope = sample.data;
        emgPort.write();
        return true;
    };
    bool interruptModule()
    {
        emgPort.interrupt();
        return true;
    }
    bool close()
    {
        emgPort.close();
        return true;
    };
    double getPeriod()
    {
        return 0.05;
    };
private:
    CometaReader cometa;
    yarp::os::Publisher<ceinms_msgs_EmgData> emgPort;
    std::vector<std::string> emgChannelNames;
    yarp::os::Node node;
    int seq;
};


int main(int argc, char **argv)
{
    yarp::os::Network yarp;
    if (!yarp.checkNetwork())
    {
        std::cout << "YARP server not running!" << std::endl;
        return -1;
    }
    yarp::os::ResourceFinder rf;
    rf.setDefaultConfigFile("cometa.ini");
    rf.setDefaultContext("cometaDataAcquisition");
    rf.configure(argc, argv);
    CometaReaderModule readerModule;
    readerModule.setName("cometaReader"); //actually useless
    readerModule.runModule(rf);

    return 0;
}

