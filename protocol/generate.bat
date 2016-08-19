@echo ------generating protocol------
@echo off
protoc --proto_path=./ --cpp_out=./ game_protocol.proto
@echo on
@echo ------    successful     ------
