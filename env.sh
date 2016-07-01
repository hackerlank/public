vm=vic

shopt -s expand_aliases
alias doma='docker-machine'

doma start $vm
eval "$(doma env $vm)"

