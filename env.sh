vm=vic

shopt -s expand_aliases
alias doma='docker-machine'

doma start $vm
eval "$(doma env $vm)"

docker run -i --volumes-from=7fde94b9f555 docker_compiler
#docker run -it -v `pwd`:/var/src -v `pwd`/../private/docker/data/thirdparts:/var/data docker_compiler
