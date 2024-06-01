#set shellc file
SHELLC="../shellc"

err() 
{
    if [ $? != 0 ];then 
        echo "ERROR: $1"
        exit
    fi
}

ok()
{
    if [ $? != 0 ];then 
        echo "ERROR: $1"
    else
        echo "OK: $1"
    fi    
}

one()
{    
    if [ -f ./$1.c ]; then    
        cc $1.c -o $1_$2.x; err "compile $1_$2.x" 
        if [ -f ./$1_$2.x ]; then
            ./$1_$2.x ; ok "run $1_$2.x" 
        fi 
    fi
    echo "Press any key to continue"
    read p
    rm -f  $1.c $1_$2.x
    clear
}

exe()
{    
    if [ -f ./$1.c ]; then    
        cc $1.c -o $1_$2.x; err "compile $1_$2.x" 
        if [ -f ./$1_$2.x ]; then
            ./$1_$2.x 1 1; ok "run $1_$2.x" 
        fi 
    fi
    echo "Press any key to continue"
    read p
    rm -f  $1.c $1_$2.x
    clear
}


bat()
{
    if [ $# != 2 ] && [ $# != 3 ];then 
        echo "Error sample:$@"
        exit 
    fi
    if [ $# = 2 ] || [ $# = 3 ]; then
        echo "----[sample1:shellc $1 $2]----"
        rm -f $2.c
        $SHELLC $1 $2 ; ok "create $2.c"
        exe $2 u
        
        echo "----[sample2:shellc $1 $2 -t]----"
        rm -f $2.c
        $SHELLC $1 $2 -t; ok "create $2.c"
        exe $2 t
        
        echo "----[sample3:shellc $1 $2 -t -s]----"
        rm -f $2.c
        $SHELLC $1 $2 -t -s; ok "create $2.c"
        exe $2 s
        
    fi 
        
    if [ $# = 3 ]; then
        echo "----[sample4:shellc $1 $2 -f $3]----"
        rm -f $2.c
        $SHELLC $1 $2 -f $3; ok "create $2.c"
        exe $2 uf
        
        echo "----[sample5:shellc $1 $2 -t -f $3]----"
        rm -f $2.c
        $SHELLC $1 $2 -t -f $3; ok "create $2.c"
        exe $2 tf
        
        echo "----[sample6:shellc $1 $2 -t -s -f $3]----"
        rm -f $2.c
        $SHELLC $1 $2 -t -s -f $3; ok "create $2.c"
        exe $2 sf
    fi
}

run()
{
    if [ $# != 2 ] && [ $# != 3 ];then 
        echo "Error sample:$@"
        exit 
    fi
    if [ $# = 2 ] || [ $# = 3 ]; then
        echo "----[sample1:shellc $1 $2]----"
        rm -f $2.c
        $SHELLC $1 $2 ; ok "create $2.c"
        exe $2 u
        
        echo "----[sample2:shellc $1 $2 -t]----" 
        rm -f $2.c  
        $SHELLC $1 $2 -t; ok "create $2.c"
        exe $2 t
        
    fi 
        
    if [ $# = 3 ]; then
        echo "----[sample3:shellc $1 $2 -f $3]----"
        rm -f $2.c  
        $SHELLC $1 $2 -f $3; ok "create $2.c"
        exe $2 uf
        
        echo "----[sample4:shellc $1 $2 -t -f $3]----"
        rm -f $2.c  
        $SHELLC $1 $2 -t -f $3; ok "create $2.c"
        exe $2 tf
        
    fi
}

saf ()
{
    if [ $# != 2 ] && [ $# != 3 ];then 
        echo "Error sample:$@"
        exit 
    fi
    if [ $# = 2 ] || [ $# = 3 ]; then
        echo "----[safe1:shellc $1 $2 -t -s]----"
        rm -f $2.c
        $SHELLC $1 $2 -t -s; ok "create $2.c"
        one $2 s
        
    fi 
        
    if [ $# = 3 ]; then
        echo "----[safe2:shellc $1 $2 -t -s -f $3]----"
        rm -f $2.c
        $SHELLC $1 $2 -t -s -f $3; ok "create $2.c"
        one $2 sf
    fi
}

clear
echo "===============run sample start==============="

bat sh test.sh

bat dash test.sh 

bat bsh test.sh
#only safe mode is ok

bat Rsh test.sh
#only safe mode is ok

bat bash test.sh BASH
#fix-arg0 support bash5.0 or above

bat zsh test.sh ZSH
#Red Hat 7.8 not support fix-arg0, but using safe mode is ok    
                                       

bat ksh test.sh

bat sh test.fix fix.txt 

#The following does not support safe mode

run tsh test.csh

run tcsh test.csh

run tclsh test.tcl

run csh test.csh
#AIX7.1 SCO_UNIX5.0 not support csh

run lua test.lua LUA

run perl test.pl PERL

run python test.py PYTHON

run python3 test.py PYTHON

run node test.js JAVASCRIPT

run Rscript test.R

run ruby test.rb RUBY

run php test.php 


#The following support safe mode, but not support parameters and human-computer interaction
saf tsh test_safe.csh

saf tcsh test_safe.csh

saf tclsh test_safe.tcl

saf csh test_safe.csh

saf lua test_safe.lua LUA

saf perl test_safe.pl PERL

saf python test_safe.py PYTHON

saf python3 test_safe.py PYTHON

saf node test_safe.js JAVASCRIPT

saf ruby test_safe.rb RUBY

saf php test_safe.php 

echo "===============run sample complete==============="

