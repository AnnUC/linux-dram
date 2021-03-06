#include <linux/dram_interface.h>
#include <linux/module.h>
#include <linux/mm_types.h>
#include <linux/sched.h>
#include <asm/current.h>
#include <linux/slab.h>

#define NUM_HEAP_IDENTS 10

static traverse_func_ptr traverse_funcs[NUM_HEAP_IDENTS];

void register_traverse(int identifier, traverse_func_ptr traverse) {
	traverse_funcs[identifier] = traverse;
}
EXPORT_SYMBOL(register_traverse);


// pending testing
asmlinkage long sys_register_heap_info (int mem_allocator_identifier, 
										void* arena_start_ptr, 
										void* subheap_start_ptr, 
										size_t subheap_size, 
										int* new_error_info_flag) {

	struct mm_struct* cur_mm;
	struct heap_info* new_info;

	cur_mm = current->mm;
	
	

	new_info = kmalloc(sizeof(struct heap_info), GFP_KERNEL);
	new_info->identifier = mem_allocator_identifier;
	new_info->heapseg_start_ptr = subheap_start_ptr;
	new_info->size = subheap_size;
	new_info->new_error_info_flag = new_error_info_flag;
	new_info->next = NULL;


	

	struct heap_info* last_info;
	last_info = NULL;

	down_write(&cur_mm->heap_info_lock);

	while (last_info && last_info->next) {
		last_info = last_info->next;
	}

	if (!last_info) {
		cur_mm->heap_info = new_info;
	} else {
		last_info->next = new_info;
	}

	up_write(&cur_mm->heap_info_lock);

	return 0;

}


// solely for testing purposes
asmlinkage long sys_test_traverse (void* arena_start_ptr,  size_t VpageNO, size_t* len) {
	struct mm_struct* cur_mm;
	int ident;

	cur_mm = current->mm;

	down_read(&cur_mm->heap_info_lock);

	if (!cur_mm->heap_info) {
		printk(KERN_WARNING "No Registered Heap!");
	}
	
	ident = cur_mm->heap_info->identifier;

	up_read(&cur_mm->heap_info_lock);

	struct free_chunk_info* ret =  traverse_funcs[ident](arena_start_ptr, VpageNO, len);

	// do something here yourself

	return 0;

}

asmlinkage long sys_update_heap_info  (int mem_allocator_identifier, void* arena_start_ptr, size_t arena_size) {
	// stubbed out 
}

asmlinkage long sys_get_faulty_address_info (void** buf, size_t arena_start_addr) {
	// stubbed out 
}

asmlinkage long sys_reserve_header (void* vaddr, size_t len) {
	// stubbed out 
}  
