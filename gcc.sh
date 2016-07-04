vm=vic

shopt -s expand_aliases
alias doma='docker-machine'

doma start $vm
eval "$(doma env $vm)"

echo export DEPENDENCIES=/var/data/sdks
echo then run make

# run this at the first time
#docker run -v `pwd`:/var/data --name=data_volume ubuntu:16.04
os=`uname -s`
if [ "Linux"=$os ]
then
	docker run -it --volumes-from=data_volume docker_gcc
else
	docker run -i --volumes-from=data_volume docker_gcc
fi

