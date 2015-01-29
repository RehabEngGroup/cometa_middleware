// This is an automatically generated file.
// Generated from this ceinms_msgs_ResetTimerReply.msg definition:
// Instances of this class can be read and written with YARP ports,
// using a ROS-compatible format.

#ifndef YARPMSG_TYPE_ceinms_msgs_ResetTimerReply
#define YARPMSG_TYPE_ceinms_msgs_ResetTimerReply

#include <string>
#include <vector>
#include <yarp/os/Wire.h>
#include <yarp/os/idl/WireTypes.h>

class ceinms_msgs_ResetTimerReply : public yarp::os::idl::WirePortable {
public:
  bool ok;

  ceinms_msgs_ResetTimerReply() {
  }

  bool readBare(yarp::os::ConnectionReader& connection) {
    // *** ok ***
    if (!connection.expectBlock((char*)&ok,1)) return false;
    return !connection.isError();
  }

  bool readBottle(yarp::os::ConnectionReader& connection) {
    connection.convertTextMode();
    yarp::os::idl::WireReader reader(connection);
    if (!reader.readListHeader(1)) return false;

    // *** ok ***
    ok = reader.expectInt();
    return !connection.isError();
  }

  bool read(yarp::os::ConnectionReader& connection) {
    if (connection.isBareMode()) return readBare(connection);
    return readBottle(connection);
  }

  bool writeBare(yarp::os::ConnectionWriter& connection) {
    // *** ok ***
    connection.appendBlock((char*)&ok,1);
    return !connection.isError();
  }

  bool writeBottle(yarp::os::ConnectionWriter& connection) {
    connection.appendInt(BOTTLE_TAG_LIST);
    connection.appendInt(1);

    // *** ok ***
    connection.appendInt(BOTTLE_TAG_INT);
    connection.appendInt((int)ok);
    connection.convertTextMode();
    return !connection.isError();
  }

  bool write(yarp::os::ConnectionWriter& connection) {
    if (connection.isBareMode()) return writeBare(connection);
    return writeBottle(connection);
  }

  // This class will serialize ROS style or YARP style depending on protocol.
  // If you need to force a serialization style, use one of these classes:
  typedef yarp::os::idl::BareStyle<ceinms_msgs_ResetTimerReply> rosStyle;
  typedef yarp::os::idl::BottleStyle<ceinms_msgs_ResetTimerReply> bottleStyle;

  // Give source text for class, ROS will need this
  yarp::os::ConstString getTypeText() {
    return "";
  }

  // Name the class, ROS will need this
  yarp::os::Type getType() {
    yarp::os::Type typ = yarp::os::Type::byName("ceinms_msgs/ResetTimerReply","ceinms_msgs/ResetTimerReply");
    typ.addProperty("md5sum",yarp::os::Value("6f6da3883749771fac40d6deb24a8c02"));
    typ.addProperty("message_definition",yarp::os::Value(getTypeText()));
    return typ;
  }
};

#endif
