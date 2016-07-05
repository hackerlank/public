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
	docker run -it --net=host --volumes-from=data_volume docker_haproxy
else
        docker run -i --net=host --volumes-from=data_volume ubuntu:16.04 /var/data/server/server
fi

