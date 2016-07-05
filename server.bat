REM alias
@doskey ls=dir /w /oe $*
@doskey rm=del $*
@doskey mv=move $*
@doskey cp=copy $*
@doskey cat=type $*
@doskey clear=cls
@doskey doma=docker-machine $*
@doskey doco=docker-compose $*

docker-machine start vic
@FOR /f "tokens=*" %%i IN ('docker-machine env vic') DO @%%i

REM run this at the first time
REM docker run -v /mnt/work:/var/data --name=data_volume docker_gcc
docker run -it --net=host --volumes-from=data_volume ubuntu:16.04 /var/data/server/server

