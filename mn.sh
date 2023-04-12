function CLEAR_IO(){
    cd io/submission
    cur_dir=1
    while [ -d $cur_dir ];
    do
        cd $cur_dir
        for file in ./*
        do
            if [[ ! "$file" = './main.'* ]]; then
                rm -f $file
            fi
        done
        cd ..
        cur_dir=`expr $cur_dir + 1`
    done
}

function BUILD(){
    if [ ! -d "target" ];then
        mkdir ./target
    fi
    cd target
    cmake ..
    make
    if [[ "$1" = "-b" ]];then
        cd ..
    fi
}

function TEST_ALL(){
    BUILD
    ./utilstest
    sudo ./ojtest
    cd ..
}

function CLEAR_TARGET(){
    if [ ! -d "target" ]; then
        mkdir target
    else
        cd target
        rm -rf *
        cd ..
    fi
}

arg=$1
if [ $arg ]; then
    case "$arg" in 
        t)
            TEST_ALL
            ;;
        ci|clear-io)
            CLEAR_IO
            ;;
        ct|clear|clear-target)
            CLEAR_TARGET
            ;;
        b|build)
            BUILD -b
            ;;
        *)
            echo "Error: wrong argument: '$1'"
            exit 1
    esac
else
    echo 'Error: no argument.'
    exit 1
fi