CFLAGS = -g -w
lab12lvcN32451: lab12lvcN32451.c
all:
	gcc -c walk_tree_and_switch_module.c
	ar r lib_walk_tree_and_switch_module.a walk_tree_and_switch_module.o
	
	gcc -c find_plugins.c
	ar r lib_find_plugins.a find_plugins.o
	
	gcc -fPIC -shared find_string_target_in_hex.c -o lib_find_string_target_in_hex.so
	gcc -fPIC -shared find_string_target.c -o lib_find_string_target.so
	gcc $(CFLAGS)  -fPIC -shared  liblvcN32451.c -o liblvcN32451.so 
	 
	gcc -g -o lab12lvcN32451 -Wall -Wextra -Werror -O3 -Wl,--no-as-needed -ldl lab12lvcN32451.c lib_walk_tree_and_switch_module.a lib_find_plugins.a
	find . -name '*.o' -delete

clean:
	find . -name 'lib_find_plugins.a' -delete
	find . -name 'lib_walk_tree_and_switch_module.a' -delete
	find . -name 'lib_find_string_target_in_hex.so' -delete
	find . -name 'lib_find_string_target.so' -delete
	find . -name 'liblvcN32451.so' -delete
	find . -name '*.o' -delete
	find . -name 'lab12lvcN32451' -delete
	
