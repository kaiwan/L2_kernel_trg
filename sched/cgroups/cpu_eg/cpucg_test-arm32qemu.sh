#!/bin/sh                                                              
# A quick test case for (CPU) CGroups.                                 
# cpucg_test.sh                                                        
# (c) Kaiwan NB, kaiwanTECH                                            
#                                                                      
CGMNT=cgroupfs                                                         
OUT1=1stjob.txt                                                        
OUT2=2ndjob.txt                                                        
                                                                       
cpu_resctrl_try()                                                      
{                                                                      
echo "[+] Running j1 and j2 now ..."                                   
#--- Run a job j1                                                      
rm -f j1                                                               
ln -sf ${TD}/simp.sh j1                                                
./j1 1 >${OUT1} &                                                      
j1pid=$(ps -A|grep j1|head -n1|awk '{print $1}')                       
                                                                       
#--- Create a group called 'g1', put j1 there making cpu share v small 
rm -rf ${CGMNT}/cpu/g1                                                 
mkdir ${CGMNT}/cpu/g1                                                  
echo ${j1pid} > ${CGMNT}/cpu/g1/tasks                                  
echo 52 > ${CGMNT}/cpu/g1/cpu.shares    # ~5%                          
                                                                       
#--- Run a job j2                                                      
rm -f j2                                                               
ln -sf ${TD}/simp.sh j2                                                
./j2 500 >${OUT2} &                                                    
j2pid=$(ps -A|grep j2|head -n1|awk '{print $1}')                      
#--- Create a group called 'g2', put j2 there making cpu share large   
rm -rf ${CGMNT}/cpu/g2                                                 
mkdir ${CGMNT}/cpu/g2                                                  
echo ${j2pid} > ${CGMNT}/cpu/g2/tasks                                  
echo 3096 > ${CGMNT}/cpu/g2/cpu.shares    # ~75%                       
                                                                       
}                                                                      
                                                                       
setup_cgs()                                                            
{                                                                      
# Already setup?                                                       
mount |grep '${CGMNT}' > /dev/null && return                           
#mount |grep 'type cgroup' > /dev/null && return                       
                                                                       
echo                                                                   
echo "[+] ---Setup and mount Cgroup fs's ..."                          
[ ! -d ${CGMNT} ] && mkdir ${CGMNT}                                    
                                                                       
# bash array support not available in ash!                             
for d in ${CGMNT}/cpu ${CGMNT}/cpuacct ${CGMNT}/cpuset ${CGMNT}/freezer
do                                                                     
  db=$(basename $d)                                                    
  #echo "${db}"                                                        
  [ ! -d $db ] && mkdir $db 2>/dev/null                                
  mcmd="mount -t cgroup -o ${db} none ./${CGMNT}/${db}"                
  echo "${mcmd}"
  eval ${mcmd} || {
    echo "mount failed, aborting..."
    rm -rf ${CGMNT}; rmdir ${db}         
    exit 1
  }
done                                                                   
mount |grep ${CGMNT}     
cd ${TD}                                                               
                                                                       
# Kill any stale instances of job1 and job2                            
j1pid=$(ps -A|grep j1|head -n1|awk '{print $1}')                       
[ ! -z ${j1pid} ] && kill ${j1pid}                                     
j2pid=$(ps -A|grep j2|head -n1|awk '{print $1}')                       
[ ! -z ${j2pid} ] && kill ${j2pid}                                     
rm -f ${OUT1} ${OUT2}                                                  
}                                                                      
                                                                       
                                                                       
### "main" here                                                        
                                                                       
[ $(id -u) -ne 0 ] && {                                                
        echo "$0: need root."                                          
        exit 1                                                         
}                                                                      
echo "[+] ---Cgroup kernel support:"                                   
[ -f /proc/config.gz ] && {                                            
  zcat /proc/config.gz |grep -i cgroup || {                            
       echo "$0: Cgroup support not builtin to kernel?? Aborting..."   
       exit 1                                                          
  }                                                                    
}                                                                      
                                                                       
TD=$(pwd)                                                              
                                                                       
setup_cgs                                                              
cpu_resctrl_try   
