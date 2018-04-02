show_runnable/rq.sh

Simple shell script that helps us see the CFS runqueues being manipulated...

Usage:
------

$ ./rq.sh 
Usage: ./rq.sh 0|1
 0 => show Only our three runnable/running processes t1, t2, t3
 1 => show system-wide runnable/running processes 
$ ./rq.sh 0

            task   PID         tree-key  switches  prio     exec-runtime         sum-exec        sum-sleep       Task Group
-------------------------------------------------------------------------------------------------------------------------------------
R             t3  1987   1239609.260059       646   120   1239609.260059        74.800633       435.434613 0 /autogroup-329
R             t3  1987   1240177.646328      1007   120   1240177.646328       117.246375       646.382100 0 /autogroup-329
R             t3  1987   1241311.855841      1728   120   1241311.855841       202.644682      1068.813948 0 /autogroup-329
R             t3  1987   1241875.994656      2087   120   1241875.994656       245.283156      1279.982995 0 /autogroup-329
R             t2  1986   1242439.905870      2447   120   1242439.905870       287.107495      1490.694039 0 /autogroup-329
R             t3  1987   1243571.227588      3170   120   1243571.227588       372.996689      1912.998723 0 /autogroup-329
R             t2  1986   1244135.974022      3526   120   1244135.974022       414.978844      2124.373590 0 /autogroup-329
R             t3  1987   1245839.032275      4610   120   1245839.032275       542.043246      2759.386630 0 /autogroup-329
^C$  


Notice how the 3rd column, which is 'tree-key', which is actually the 
se->vruntime, changes. It's _not_ necessarily a monotonically increasing value.

