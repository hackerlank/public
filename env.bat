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

docker run -v project:/var/data --name=data_volume docker_gcc6
docker run -it --volumes-from=data_volume docker_gcc6 /var/data/bashrc.sh

