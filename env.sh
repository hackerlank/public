vm=vic

shopt -s expand_aliases
alias doma='docker-machine'

doma start $vm
eval "$(doma env $vm)"

docker run -v `pwd`:/var/data --name=data_volume docker_gcc6
docker run -it --volumes-from=data_volume docker_gcc6 /var/data/bashrc.sh

