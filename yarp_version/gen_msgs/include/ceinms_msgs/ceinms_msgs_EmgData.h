// This is an automatically generated file.
// Generated from this ceinms_msgs_EmgData.msg definition:
//   Header header
//   
//   string[] name
//   float64[] envelope
//   float64[] raw
// Instances of this class can be read and written with YARP ports,
// using a ROS-compatible format.

#ifndef YARPMSG_TYPE_ceinms_msgs_EmgData
#define YARPMSG_TYPE_ceinms_msgs_EmgData

#include <string>
#include <vector>
#include <yarp/os/Wire.h>
#include <yarp/os/idl/WireTypes.h>
#include "TickTime.h"
#include "Header.h"

class ceinms_msgs_EmgData : public yarp::os::idl::WirePortable {
public:
  Header header;
  std::vector<std::string> name;
  std::vector<yarp::os::NetFloat64> envelope;
  std::vector<yarp::os::NetFloat64> raw;

  ceinms_msgs_EmgData() {
  }

  bool readBare(yarp::os::ConnectionReader& connection) {
    // *** header ***
    if (!header.read(connection)) return false;

    // *** name ***
    int len = connection.expectInt();
    name.resize(len);
    for (int i=0; i<len; i++) {
      int len2 = connection.expectInt();
      name[i].resize(len2);
      if (!connection.expectBlock((char*)name[i].c_str(),len2)) return false;
    }

    // *** envelope ***
    len = connection.expectInt();
    envelope.resize(len);
    if (!connection.expectBlock((char*)&envelope[0],sizeof(yarp::os::NetFloat64)*len)) return false;

    // *** raw ***
    len = connection.expectInt();
    raw.resize(len);
    if (!connection.expectBlock((char*)&raw[0],sizeof(yarp::os::NetFloat64)*len)) return false;
    return !connection.isError();
  }

  bool readBottle(yarp::os::ConnectionReader& connection) {
    connection.convertTextMode();
    yarp::os::idl::WireReader reader(connection);
    if (!reader.readListHeader(4)) return false;

    // *** header ***
    if (!header.read(connection)) return false;

    // *** name ***
    if (connection.expectInt()!=(BOTTLE_TAG_LIST|BOTTLE_TAG_STRING)) return false;
    int len = connection.expectInt();
    name.resize(len);
    for (int i=0; i<len; i++) {
      int len2 = connection.expectInt();
      name[i].resize(len2);
      if (!connection.expectBlock((char*)name[i].c_str(),len2)) return false;
    }

    // *** envelope ***
    if (connection.expectInt()!=(BOTTLE_TAG_LIST|BOTTLE_TAG_DOUBLE)) return false;
    len = connection.expectInt();
    envelope.resize(len);
    for (size_t i=0; i<len; i++) {
      envelope[i] = (yarp::os::NetFloat64)connection.expectDouble();
    }

    // *** raw ***
    if (connection.expectInt()!=(BOTTLE_TAG_LIST|BOTTLE_TAG_DOUBLE)) return false;
    len = connection.expectInt();
    raw.resize(len);
    for (size_t i=0; i<len; i++) {
      raw[i] = (yarp::os::NetFloat64)connection.expectDouble();
    }
    return !connection.isError();
  }

  bool read(yarp::os::ConnectionReader& connection) {
    if (connection.isBareMode()) return readBare(connection);
    return readBottle(connection);
  }

  bool writeBare(yarp::os::ConnectionWriter& connection) {
    // *** header ***
    if (!header.write(connection)) return false;

    // *** name ***
    connection.appendInt(name.size());
    for (size_t i=0; i<name.size(); i++) {
      connection.appendInt(name[i].length());
      connection.appendExternalBlock((char*)name[i].c_str(),name[i].length());
    }

    // *** envelope ***
    connection.appendInt(envelope.size());
    connection.appendExternalBlock((char*)&envelope[0],sizeof(yarp::os::NetFloat64)*envelope.size());

    // *** raw ***
    connection.appendInt(raw.size());
    connection.appendExternalBlock((char*)&raw[0],sizeof(yarp::os::NetFloat64)*raw.size());
    return !connection.isError();
  }

  bool writeBottle(yarp::os::ConnectionWriter& connection) {
    connection.appendInt(BOTTLE_TAG_LIST);
    connection.appendInt(4);

    // *** header ***
    if (!header.write(connection)) return false;

    // *** name ***
    connection.appendInt(BOTTLE_TAG_LIST|BOTTLE_TAG_STRING);
    connection.appendInt(name.size());
    for (size_t i=0; i<name.size(); i++) {
      connection.appendInt(name[i].length()+1);
      connection.appendExternalBlock((char*)name[i].c_str(),name[i].length()+1);
    }

    // *** envelope ***
    connection.appendInt(BOTTLE_TAG_LIST|BOTTLE_TAG_DOUBLE);
    connection.appendInt(envelope.size());
    for (size_t i=0; i<envelope.size(); i++) {
      connection.appendDouble((double)envelope[i]);
    }

    // *** raw ***
    connection.appendInt(BOTTLE_TAG_LIST|BOTTLE_TAG_DOUBLE);
    connection.appendInt(raw.size());
    for (size_t i=0; i<raw.size(); i++) {
      connection.appendDouble((double)raw[i]);
    }
    connection.convertTextMode();
    return !connection.isError();
  }

  bool write(yarp::os::ConnectionWriter& connection) {
    if (connection.isBareMode()) return writeBare(connection);
    return writeBottle(connection);
  }

  // This class will serialize ROS style or YARP style depending on protocol.
  // If you need to force a serialization style, use one of these classes:
  typedef yarp::os::idl::BareStyle<ceinms_msgs_EmgData> rosStyle;
  typedef yarp::os::idl::BottleStyle<ceinms_msgs_EmgData> bottleStyle;

  // Give source text for class, ROS will need this
  yarp::os::ConstString getTypeText() {
    return "Header header\n\
\n\
string[] name\n\
float64[] envelope\n\
float64[] raw\n================================================================================\n\
MSG: std_msgs/Header\n\
uint32 seq\n\
time stamp\n\
string frame_id";
  }

  // Name the class, ROS will need this
  yarp::os::Type getType() {
    yarp::os::Type typ = yarp::os::Type::byName("ceinms_msgs/EmgData","ceinms_msgs/EmgData");
    typ.addProperty("md5sum",yarp::os::Value("90e54c5a4cfd44150ec0b260907dae98"));
    typ.addProperty("message_definition",yarp::os::Value(getTypeText()));
    return typ;
  }
};

#endif
