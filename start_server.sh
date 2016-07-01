vm=vic

shopt -s expand_aliases
alias doma='docker-machine'

doma start $vm
eval "$(doma env $vm)"

# run this at the first time
#docker run -v `pwd`:/var/data --name=data_volume ubuntu:16.04
docker run -i --volumes-from=data_volume docker_server

