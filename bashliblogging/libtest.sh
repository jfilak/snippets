function setup_logging
{
    exec 1> >(tee -a $1) 2>&1
}
