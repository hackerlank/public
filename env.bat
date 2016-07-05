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
