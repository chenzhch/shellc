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


exe()
{    
    if [ -f ./$1.c ]; then    
        cc $1.c  -o $1_$2.x; err "compile $1_$2.x"
        #cc $1.c -Wall -o $1_$2.x; err "compile $1_$2.x"  
        if [ -f ./$1_$2.x ]; then
            ./$1_$2.x 1 2; ok "run $1_$2.x" 
        fi 
    fi
    echo "Press any key to continue"
    read p
    rm -f  $1.c $1_$2.x
    clear
}

one()
{    
    if [ -f ./$1.c ]; then    
        cc $1.c  -o $1_$2.x; err "compile $1_$2.x" 
        #cc $1.c -Wall -o $1_$2.x; err "compile $1_$2.x" 
        if [ -f ./$1_$2.x ]; then
            ./$1_$2.x ; ok "run $1_$2.x" 
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
        
        echo "----[sample6:shellc $1 $2 -ts -f $3]----"
        rm -f $2.c
        $SHELLC $1 $2 -ts -f $3; ok "create $2.c"
        exe $2 sf
    fi
}

fix()
{
          
    if [ $# = 3 ]; then
        echo "----[fix-file1:shellc $1 $2 -e $3]----"
        rm -f $2.c
        $SHELLC $1 $2 -e $3; ok "create $2.c"
        exe $2 ue
        
        echo "----[fix-file2:shellc $1 $2 -t -e $3]----"
        rm -f $2.c
        $SHELLC $1 $2 -t -e $3; ok "create $2.c"
        exe $2 te
        
        echo "----[fix-file3:shellc $1 $2 -ts -e $3]----"
        rm -f $2.c
        $SHELLC $1 $2 -ts -e $3; ok "create $2.c"
        exe $2 se
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

inp ()
{
    if [ $# != 2 ] && [ $# != 3 ];then 
        echo "Error sample:$@"
        exit 
    fi

    if [ $# = 2 ] ; then
        echo "----[input1:shellc $1 $2 -t ]----"
        rm -f $2.c
        $SHELLC $1 $2 -t ; ok "create $2.c"
        exe $2 fi

        echo "----[input2:shellc $1 $2 -t -s ]----"
        rm -f $2.c
        $SHELLC $1 $2 -t -s ; ok "create $2.c"
        exe $2 si
        
    fi 
            
    if [ $# = 3 ]; then
        echo "----[input1:shellc $1 $2 -t -f $3]----"
        rm -f $2.c
        $SHELLC $1 $2 -t -f $3; ok "create $2.c"
        exe $2 fi

        echo "----[input2:shellc $1 $2 -t -s -f $3]----"
        rm -f $2.c
        $SHELLC $1 $2 -t -s -f $3; ok "create $2.c"
        exe $2 si
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

bat fish test.fish FISH

bat tcsh test.csh CSH

bat csh test.csh CSH

bat tclsh test.tcl TCLSH

bat fish test.fish FISH

bat perl test.pl PERL

bat lua test.lua LUA

bat python test.py PYTHON

bat python3 test.py PYTHON

bat node test.js JAVASCRIPT

bat ruby test.rb RUBY

bat php test.php PHP 

#bat rc test.rc RC

fix sh test.fix fix.txt 

run Rscript test.R

#The following test parameters and human-computer interaction 

inp sh test_input.sh

inp fish test_input.fish FISH

inp lua test_input.lua LUA

inp perl test_input.pl PERL

inp python test_input.py PYTHON

inp python3 test_input.py PYTHON

inp node test_input.js JAVASCRIPT

inp ruby test_input.rb RUBY

inp tclsh test_input.tcl TCLSH

inp php test_input.php PHP

echo "===============run sample complete==============="

