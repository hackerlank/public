vm=vic

shopt -s expand_aliases
alias doma='docker-machine'

doma start $vm
eval "$(doma env $vm)"

# run this at the first time
#docker run -v `pwd`:/var/data --name=data_volume docker_gcc6
docker run -i --volumes-from=data_volume docker_gcc6
echo export DEPENDENCIES=/var/data/public/sdks
echo then run make

