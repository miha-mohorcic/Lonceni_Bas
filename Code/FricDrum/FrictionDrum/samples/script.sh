#!/bin/bash

orig=down; 
next=.wav;

for i in `seq 1 24`;
do
	new=$orig$i.wav;
	p=`bc <<< "scale=2; ${i}/3"`;
	echo $p;
	soundstretch ${orig}0.wav $new -pitch=$p;
done
