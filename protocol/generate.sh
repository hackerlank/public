protoc --proto_path=./ --cpp_out=./ --csharp_out=../ImmortalCard/Assets/Scripts/ protocol.proto
cp -rf ../ImmortalCard/Assets/Scripts/Protocol.cs ../ImmortalPay/Assets/Scripts/
