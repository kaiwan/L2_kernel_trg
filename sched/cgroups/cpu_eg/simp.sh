#!/bin/sh
# 1st param is starting #
[ $# -ne 1 ] && {
 echo "Usage: $0 <starting#>"
 exit 1
}
max=9999
[ $1 -ge $max ] && {
 echo "Param $1 >= max ($max)"
 exit 1
}

delay_loop()
{                    
for x in $(seq 1 10); do
  for y in $(seq 1 200); do
    z=$(((y+10000)*x/y))    
  done                      
done                        
}                    
                     
i=$1                 
while [ $i -le $max ]
do                   
  echo -n "$i "
  i=$((i+1))   
  delay_loop
done        

