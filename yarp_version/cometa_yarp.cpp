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

    int emgEnabledChanNum;
    // Bandpass filter for raw EMG signal
    std::vector<VOSL::Filter::Filter<double> > filters;
    // Lowpass filter for rectified EMG signal
    std::vector<VOSL::Filter::Filter<double> > filtersLE;
    // Maximum value (so far) for envelopes, used for normalization
    std::vector<double> maxEnvs;
    bool newEpoch;

public:
    //only works if called before initialize
    void setTriggerIn(bool on = true)
    {
        if (on)
            trgMode = DAQ_TRG_IN_ACTIVE_HI;
        else
            trgMode = DAQ_TRG_OUT_ENABLE;
    }
    int initialize(std::vector<int> enabledChans)
    {
        //step 1: create Wave device
        device = CreateWaveDevice();
        if (device == NULL)
        {
            printf("\nError creating device");
            return -1;
        }

        // step 2: get the Emg and FootSw installed channels number
        if ((errcode = device->getInstalledChan(&emgInstalledChanNum, &fswInstalledChanNum)) != 0)
        {
            printf("\nError executing getInstalledChan() method - Error code= %d\n", errcode);
            DeleteWaveDevice();
            return errcode;
        }

        // step 3: configure acquisition parameters
        emgEnabledChanNum = enabledChans.size();
        //enable all the Emg channels (1= enabled, 0= disabled) 
        for (i = 0; i < emgInstalledChanNum; i++)
            (std::find(enabledChans.begin(), enabledChans.end(), i) != enabledChans.end()) ? emgChanEnableVect[i] = 1 : emgChanEnableVect[i] = 0;

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

        //high pass 30 hz
        std::vector<std::complex<double> > bCoeffHP;
        bCoeffHP.reserve(5);
        bCoeffHP.push_back(0.8841);
        bCoeffHP.push_back(-3.5364);
        bCoeffHP.push_back(5.3046);
        bCoeffHP.push_back(-3.5364);
        bCoeffHP.push_back(0.8841);
        std::vector<std::complex<double>> aCoeffHP;
        aCoeffHP.reserve(5);
        aCoeffHP.push_back(1.0000);
        aCoeffHP.push_back(-3.7538);
        aCoeffHP.push_back(5.2912);
        aCoeffHP.push_back(-3.3189);
        aCoeffHP.push_back(0.7816);
        //low pass 450 hz
        std::vector<std::complex<double> > bCoeffLP;
        bCoeffLP.reserve(5);
        bCoeffLP.push_back(0.0675);
        bCoeffLP.push_back(0.2700);
        bCoeffLP.push_back(0.4050);
        bCoeffLP.push_back(0.2700);
        bCoeffLP.push_back(0.0675);
        std::vector<std::complex<double>> aCoeffLP;
        aCoeffLP.reserve(5);
        aCoeffLP.push_back(1.0000);
        aCoeffLP.push_back(-0.3906);
        aCoeffLP.push_back(0.5343);
        aCoeffLP.push_back(-0.0842);
        aCoeffLP.push_back(0.0207);

        VOSL::Filter::Polynomial<double> bHP(bCoeffHP);
        VOSL::Filter::Polynomial<double> aHP(aCoeffHP);
        VOSL::Filter::Polynomial<double> bLP(bCoeffLP);
        VOSL::Filter::Polynomial<double> aLP(aCoeffLP);

        VOSL::Filter::TransferFunction<double> emgFilterTF =
            VOSL::Filter::TransferFunction<double>(bLP, aLP, 2000) * VOSL::Filter::TransferFunction<double>(bHP, aHP, 2000);

        for (int i = 0; i < emgInstalledChanNum; ++i)
        {
            filters.push_back(VOSL::Filter::Filter<double>(emgFilterTF));
        }

        //low pass 8 hz
        for (int i = 0; i < emgInstalledChanNum; ++i)
        {
            filtersLE.push_back(VOSL::Filter::Filter<double>(VOSL::Filter::TransferFunction<double>(VOSL::Filter::butter<double>(4, 8, 2000))));
        }

        // step 3c: initialize maximum envelope values
        for (int i = 0; i < emgInstalledChanNum; ++i)
        {
            maxEnvs.push_back(-std::numeric_limits<double>::infinity());
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
        return 0;
    };

    CometaReader(){
        //initialize variables
        fswProtocolType = FSW_PROT_QUARTER_FOOT;
        trgMode = DAQ_TRG_OUT_ENABLE;
        emgInstalledChanNum = 0;
        fswInstalledChanNum = 0;
        newEpoch = true;

        //          if (initialize() != 0)
        //              exit(EXIT_FAILURE);
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



    std::vector<double> getSample()
    {
        std::vector<double> currentSample(emgEnabledChanNum);
        errcode = device->transferData(&dataBuff, &dataNum, &isAcqRunning, &startFromTrg, &stopFromTrg,
            &firstSample, &lastSample);
        if (errcode == 0)
        {
            size_t offset = 0;
            size_t curChan = 0;
            while (offset < dataNum - emgEnabledChanNum)
            {
                for (curChan = 0; curChan < emgEnabledChanNum; curChan++)
                {
                    double sampleValue = filtersLE.at(curChan).filter(abs(filters.at(curChan).filter(dataBuff[offset + curChan])));
                    if (sampleValue>maxEnvs.at(curChan))
                        maxEnvs.at(curChan) = sampleValue;
                    currentSample.at(curChan) = sampleValue / maxEnvs.at(curChan);
                }
                offset += emgEnabledChanNum;
            }
            return currentSample;
        }
        return std::vector<double>();
    };
};


class CometaReaderModule : public yarp::os::RFModule
{
public:
    CometaReaderModule() : node("/cometaReader"){};
    bool configure(yarp::os::ResourceFinder& rf)
    {
        emgPort.topic("/emgData");

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
            yWarning("Could not connect to ROS parameter server to retrieve list of emgNames - chans");
            return false;
        }
        if (reply.get(0).asInt() != 1)
        {
            yWarning("ROS parameter server did not successfully provide list of emgNames ");
            return false;
        }
        emgChannelNames.clear();
        yarp::os::Bottle* names = reply.get(2).asList();
        for (int bt = 0; bt < names->size(); ++bt)
            emgChannelNames.push_back(names->get(bt).asString());

        // get channel numbers
        query.clear();
        query.addString("getParam");
        query.addString("/cometaReader");
        query.addString("emgChans");
        reply.clear();
        ok = yarp::os::Network::write(yarp::os::Contact::byName("/ros"), query, reply, style);
        if (!ok)
        {
            yWarning("Could not connect to ROS parameter server to retrieve list of emgNames - chans");
            return false;
        }
        if (reply.get(0).asInt() != 1)
        {
            yWarning("ROS parameter server did not successfully provide list of emgNames ");
            return false;
        }
        std::vector<int> enabledChannels;
        yarp::os::Bottle* chans = reply.get(2).asList();
        for (int bt = 0; bt < chans->size(); ++bt)
            enabledChannels.push_back(chans->get(bt).asInt());

        if (rf.check("trigIn"))
            cometa.setTriggerIn(true);
        return cometa.initialize(enabledChannels) == 0;
    };
    bool updateModule(){
        std::vector<double> sampleVector = cometa.getSample();
        ceinms_msgs_EmgData& sampleOnPort = emgPort.prepare();
        double time = yarp::os::Time::now();
        sampleOnPort.header.stamp.sec = time;
        sampleOnPort.header.stamp.nsec = (time - sampleOnPort.header.stamp.sec) * 1000000000;
        sampleOnPort.name = emgChannelNames;
        sampleOnPort.envelope = sampleVector;
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

