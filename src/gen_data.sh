#!/bin/sh

# bash gen_data.sh -b 1 1
# python3 tmp/sample_gen.py $alice_samples $bob_samples $alice_samples_file $bob_samples_file

alice_samples=
bob_samples=

alice_samples_path=
bob_samples_path=


while getopts "bu" OPTION
do
    case $OPTION in
        b)  
            for i in {12..20..2}
            do
                alice_samples=$((2**$i))
                bob_samples=$((2**$i))
                alice_samples_path="tmp/balanced_samples/${i}_alice_b_samples.txt"
                bob_samples_path="tmp/balanced_samples/${i}_bob_b_samples.txt"

                python3 tmp/sample_gen.py $alice_samples $bob_samples $alice_samples_path $bob_samples_path
            done
            ;;
        u)  
            for j in {18..22..2}
            do
                alice_samples=$((2**$j))
                alice_samples_path="tmp/unbalanced_samples/${j}_alice_u_samples.txt"
        
                for k in {10..12..2}
                do
                    bob_samples=$((2**$k))
                    bob_samples_path="tmp/unbalanced_samples/${k}_bob_u_samples.txt"

                    python3 tmp/sample_gen.py $alice_samples $bob_samples $alice_samples_path $bob_samples_path
                done
            done
            
            ;;
        ?)  echo "Wrong arguments passed"
            exit 2
            ;;
    esac
done

