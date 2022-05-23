#!/usr/bin/python3

import random
import sys
from time import time

def gen_samples(file, size):
    random.seed(time())
    sample=random.sample(range(0, (2**28)-1), size)
    f=open(file, 'w')
    for element in sample:
        f.write("%009d\n"%(element))
    f.close
    print("Write in " + file)

if __name__=='__main__':
        if len(sys.argv) > 1:
            size1= int(sys.argv[1])
            size2= int(sys.argv[2])
            # arquivo=str(input("What is the file name? >"))
            file1 = str(sys.argv[3])
            file2 = str(sys.argv[4])
            # file2 = "bob_samples.txt" 
            gen_samples(file1, size1)
            gen_samples(file2, size2)
        else:
            size1=int(input("What is the first sample size? >"))
            size2=int(input("What is the second sample size? >")) 
            # arquivo=str(input("What is the file name? >"))
            file1 = str(input("Name of the first file? >"))
            file2 = str(input("Name of the first file? >"))
            # file2 = "bob_samples.txt" 
            gen_samples(file1, size1)
            gen_samples(file2, size2)