vm=vic

shopt -s expand_aliases
alias doma='docker-machine'

doma start $vm
eval "$(doma env $vm)"

# run this at the first time
#docker run -v `pwd`:/var/data --name=data_volume docker_gcc
os=`uname -s`
if [ "Linux"=$os ]
then
	docker run -it --net=host --volumes-from=data_volume docker_tomcat /var/src/run.sh
else
	docker run -i --net=host --volumes-from=data_volume docker_tomcat /var/src/run.sh
fi

