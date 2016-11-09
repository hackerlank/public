vm=vic

shopt -s expand_aliases
alias doma='docker-machine'

doma start $vm
eval "$(doma env $vm)"

docker run -it -v `pwd`:/var/data centos6_dev
