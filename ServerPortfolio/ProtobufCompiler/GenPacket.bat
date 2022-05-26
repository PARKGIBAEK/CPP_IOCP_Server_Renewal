protoc.exe --proto_path=./ --cpp_out=./  ./Protocol.proto
IF ERRORLEVEL 1 PAUSE

XCOPY /Y Protocol.pb.h "../Common/ProtobufSource"
XCOPY /Y Protocol.pb.cc "../Common/ProtobufSource"



SET /P P=Press any key to continue