REM alias
@doskey ls=dir /w /oe $*
@doskey rm=del $*
@doskey mv=move $*
@doskey cp=copy $*
@doskey clear=cls
@doskey doma=docker-machine $*
@doskey doco=docker-compose $*

docker-machine start vic
@FOR /f "tokens=*" %%i IN ('docker-machine env vic') DO @%%i

REM run this at the first time
REM docker run -v /mnt/work:/var/data --name=data_volume docker_gcc6
docker run -it --volumes-from=data_volume docker_gcc6

echo export DEPENDENCIES=/var/data/public/sdks
echo then run make
REM /var/data/public/bashrc.sh

