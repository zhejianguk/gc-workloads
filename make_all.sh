cd gc-fed/overlay/root

cd helloworld
make clean
make
cd ..

cd helloworld_pthread
make clean
make
cd ..

cd tests_gc
make clean
make malloc
make tc_pmc
make tc_sanitiser
cd ..