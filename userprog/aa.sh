cd ..
source ./activate
cd userprog
make clean
make
cd build
#pintos -v -k -T 60 -m 20   --fs-disk=10 -p tests/userprog/exec-once:exec-once -p tests/userprog/child-imple:child-simple -- -q   -f run exec-once

pintos -v -k -T 60 -m 20   --fs-disk=10 -p tests/userprog/exec-once:exec-once -p tests/userprog/child-simple:child-simple -- -q   -f run exec-once < /dev/null 2> tests/userprog/exec-once.errors > tests/userprog/exec-once.output
perl -I../.. ../../tests/userprog/exec-once.ck tests/userprog/exec-once tests/userprog/exec-once.result
