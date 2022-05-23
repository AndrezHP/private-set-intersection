#!/bin/sh

rm -f tmp/benchmarks.txt
sudo fuser -k 8050/tcp
sudo fuser -k 8060/tcp
sudo fuser -k 8070/tcp

sleep 5

number_of_rounds=5
client_size=$2
server_size=$3
echo "compile with:" -$1
sh compile.sh -$1
net_rounds="200 50 10"

for net_round in $net_rounds 
do
    sh network_sim.sh restart $net_round
    rm -f tmp/logfile.txt
    for round in $(seq 1 $number_of_rounds)
    do
        echo "running round:" $round
        parallel -j0 ::: "./server $server_size" "./client $client_size $server_size"
        echo sleep
        sleep 1
    done
    
    preprocess=0
    transmission=0
    totaltime=0
    sending=0
    i=0
    input="tmp/logfile.txt"
    while IFS= read -r line
    do
        case $i in
        0)      ((preprocess+=$(echo $line | egrep -o '[0-9]*')))
                ;;
        1|3|5)  sending=$(echo $line | egrep -o '[0-9]*')
                ;;
        2|4|6)  ((transmission+=$(echo $line | egrep -o '[0-9]*') - $sending))
                ;;
        7)      ((totaltime+=$(echo $line | egrep -o '[0-9]*')))
                ;;
        esac
        ((i=(i+1)%8))

    done < "$input"
    
    echo "Average preprocess time:" $(($preprocess/$number_of_rounds)) 
    echo "Average transmission time:" $(($transmission/$number_of_rounds))
    echo "Average total time:" $(($totaltime/$number_of_rounds))

    printf "Results for protocol: -%s and bandwidth: %d mbit \n" $1  $net_round >> tmp/benchmarks.txt
    printf "Average preprocess time: %d ms\n" $(($preprocess/$number_of_rounds)) >> tmp/benchmarks.txt
    printf "Average transmission time: %d ms\n" $(($transmission/$number_of_rounds)) >> tmp/benchmarks.txt
    printf "Average total time: %d ms\n\n" $(($totaltime/$number_of_rounds)) >> tmp/benchmarks.txt
done

printf "Protocol: -$1: with size $2 and $3\n" >> tmp/benchmarks2.txt

i=0
let preprocess_sum=0
input="tmp/benchmarks.txt"
while IFS= read -r line
do
    case $i in
    0)      ;;
    1)      preprocess_sum=$(echo $line | egrep -o '[0-9]*')+$preprocess_sum
            ;;
    2|3|5)  ;;
    esac
    ((i=(i+1)%5))

done < "$input"

let preprocess_average=preprocess_sum/3


printf "Final Preprocess Average: $preprocess_average ms\n" >> tmp/benchmarks2.txt


IFS=', ' read -r -a array <<< "$net_rounds"
i=0
index=1
input3="tmp/benchmarks.txt"
while IFS= read -r line
do
    case $i in
    0|1|2)  ;;
    3)      online=$(echo $line | egrep -o '[0-9]*')
            echo $line
            printf "Online time ${array[$index - 1]}: %d ms\n" $online >> tmp/benchmarks2.txt
            printf "Average total time ${array[$index - 1]}: %d ms\n" $(($online+$preprocess_average)) >> tmp/benchmarks2.txt
            ((index=index+1))
            ;;
    4|5)      ;;
    
    esac
    ((i=(i+1)%5))
done < "$input"
printf "\n" >> tmp/benchmarks2.txt