// This is an automatically generated file.
// Generated from this ceinms_msgs_ResetTimer.msg definition:
//   ---
//   bool ok// Instances of this class can be read and written with YARP ports,
// using a ROS-compatible format.

#ifndef YARPMSG_TYPE_ceinms_msgs_ResetTimer
#define YARPMSG_TYPE_ceinms_msgs_ResetTimer

#include <string>
#include <vector>
#include <yarp/os/Wire.h>
#include <yarp/os/idl/WireTypes.h>
#include <ceinms_msgs/ceinms_msgs_ResetTimerReply.h>

class ceinms_msgs_ResetTimer : public yarp::os::idl::WirePortable {
public:

  ceinms_msgs_ResetTimer() {
  }

  bool readBare(yarp::os::ConnectionReader& connection) {
    return !connection.isError();
  }

  bool readBottle(yarp::os::ConnectionReader& connection) {
    connection.convertTextMode();
    yarp::os::idl::WireReader reader(connection);
    if (!reader.readListHeader(0)) return false;

    return !connection.isError();
  }

  bool read(yarp::os::ConnectionReader& connection) {
    if (connection.isBareMode()) return readBare(connection);
    return readBottle(connection);
  }

  bool writeBare(yarp::os::ConnectionWriter& connection) {
    return !connection.isError();
  }

  bool writeBottle(yarp::os::ConnectionWriter& connection) {
    connection.appendInt(BOTTLE_TAG_LIST);
    connection.appendInt(0);

    connection.convertTextMode();
    return !connection.isError();
  }

  bool write(yarp::os::ConnectionWriter& connection) {
    if (connection.isBareMode()) return writeBare(connection);
    return writeBottle(connection);
  }

  // This class will serialize ROS style or YARP style depending on protocol.
  // If you need to force a serialization style, use one of these classes:
  typedef yarp::os::idl::BareStyle<ceinms_msgs_ResetTimer> rosStyle;
  typedef yarp::os::idl::BottleStyle<ceinms_msgs_ResetTimer> bottleStyle;

  // Give source text for class, ROS will need this
  yarp::os::ConstString getTypeText() {
    return "---\n\
bool ok";
  }

  // Name the class, ROS will need this
  yarp::os::Type getType() {
    yarp::os::Type typ = yarp::os::Type::byName("ceinms_msgs/ResetTimer","ceinms_msgs/ResetTimer");
    typ.addProperty("md5sum",yarp::os::Value("d41d8cd98f00b204e9800998ecf8427e"));
    typ.addProperty("message_definition",yarp::os::Value(getTypeText()));
    return typ;
  }
};

#endif
