
#include "mem.h"
#include "stdlib.h"
#include "string.h"
#include <pthread.h>
#include <stdio.h>

// fix include
#include <os-mm.h>

static BYTE _ram[RAM_SIZE];

static struct {
	uint32_t proc;	// ID of process currently uses this page
	int index;	// Index of the page in the list of pages allocated
			// to the process.
	int next;	// The next page in the list. -1 if it is the last
			// page.
} _mem_stat [NUM_PAGES];

static pthread_mutex_t mem_lock;

void init_mem(void) {
	memset(_mem_stat, 0, sizeof(*_mem_stat) * NUM_PAGES);
	memset(_ram, 0, sizeof(BYTE) * RAM_SIZE);
	pthread_mutex_init(&mem_lock, NULL);
}

/* get offset of the virtual address */
static addr_t get_offset(addr_t addr) {
	return addr & ~((~0U) << OFFSET_LEN);
}

/* get the first layer index */
static addr_t get_first_lv(addr_t addr) {
	return addr >> (OFFSET_LEN + PAGE_LEN);
}

/* get the second layer index */
static addr_t get_second_lv(addr_t addr) {
	return (addr >> OFFSET_LEN) - (get_first_lv(addr) << PAGE_LEN);
}

/* Search for page table table from the a segment table */
static struct trans_table_t * get_trans_table(
		addr_t index, 	// Segment level index
		struct page_table_t * page_table) { // first level table
	
	/* DO NOTHING HERE. This mem is obsoleted */

	// tiem page table lev2 from page table lev1
	int i;
	for (i = 0; i < page_table->size; i++) {
		// Enter your code here
		if(page_table->table[i].v_index == index){
			return page_table->table[i].next_lv;
		}
	}
	return NULL;

}

/* Translate virtual address to physical address. If [virtual_addr] is valid,
 * return 1 and write its physical counterpart to [physical_addr].
 * Otherwise, return 0 */
static int translate(
		addr_t virtual_addr, 	// Given virtual address
		addr_t * physical_addr, // Physical address to be returned
		struct pcb_t * proc) {  // Process uses given virtual address

	/* Offset of the virtual address */
	addr_t offset = get_offset(virtual_addr);
        offset++; offset--;
	/* The first layer index */
	addr_t first_lv = get_first_lv(virtual_addr);
	/* The second layer index */
	addr_t second_lv = get_second_lv(virtual_addr);
	
	/* Search in the first level */
	struct trans_table_t * trans_table = NULL;
	trans_table = get_trans_table(first_lv, proc->page_table);
	if (trans_table == NULL) {
		return 0;
	}

	int i;
	for (i = 0; i < trans_table->size; i++) {
		if (trans_table->table[i].v_index == second_lv) {
			/* DO NOTHING HERE. This mem is obsoleted */
			return 1;
		}
	}
	return 0;	
}

addr_t alloc_mem(uint32_t size, struct pcb_t * proc) {
	pthread_mutex_lock(&mem_lock);
	addr_t ret_mem = 0;
	/* DO NOTHING HERE. This mem is obsoleted */

	uint32_t num_pages = (size % PAGE_SIZE) ? size / PAGE_SIZE :
		size / PAGE_SIZE + 1; // Number of pages we will use
	int mem_avail = 0; // We could allocate new memory region or not?

	/* First we must check if the amount of free memory in
	 * virtual address space and physical address space is
	 * large enough to represent the amount of required 
	 * memory. If so, set 1 to [mem_avail].
	 * Hint: check [proc] bit in each page of _mem_stat
	 * to know whether this page has been used by a process.
	 * For virtual memory space, check bp (break pointer).
	 * */

	 // check free memory in virtual address space
	int free_pages = 0;
	int i = 0;
	for(; i < NUM_PAGES; i++){
		if(_mem_stat[i].proc == 0){
			free_pages++;
		}
	}

	// check free memory in physical address space
	// bp: break pointer ( chi so luong page da duoc cap phat ) = con tro vung heap
	if( free_pages >= num_pages && proc->bp + num_pages * PAGE_SIZE < RAM_SIZE){
		mem_avail = 1;
	}

	if (mem_avail) {
		/* We could allocate new memory region to the process */

		ret_mem = proc->bp; // dia chi bat dau cap phat 
		proc->bp += num_pages * PAGE_SIZE;// cap nhat break pointer

		/* Update status of physical pages which will be allocated
		 * to [proc] in _mem_stat. Tasks to do:
		 * 	- Update [proc], [index], and [next] field
		 * 	- Add entries to segment table page tables of [proc]
		 * 	  to ensure accesses to allocated memory slot is
		 * 	  valid. */

		// gan trang cho process
		int j = 0;
		int allocated = 0;
		int prev_page = -1;
		for(; i < NUM_PAGES && allocated < num_pages; i++){
			if(_mem_stat[i].proc  == 0){
				_mem_stat[i].proc = proc->pid; // gan pid cho page
				_mem_stat[i].index = i; // index cua page
				_mem_stat[i].next = -1; // page cuoi cung
				allocated++;
			}
			if(prev_page != -1){
				_mem_stat[prev_page].next = i; // gan page tiep theo
			}
			prev_page = i;
		}
		_mem_stat[prev_page].next = -1; // page cuoi cung

		// update bang phan trang cua process 
		for (uint32_t i = 0; i < num_pages; i++) {
            addr_t first_lv = get_first_lv(ret_mem + i * PAGE_SIZE);
            addr_t second_lv = get_second_lv(ret_mem + i * PAGE_SIZE);

            // Tìm hoặc tạo bảng phân trang cấp 2
            struct trans_table_t *trans_table = get_trans_table(first_lv, proc->page_table);
            if (trans_table == NULL) {
                // Nếu chưa có bảng phân trang cấp 2, tạo mới
                for (int j = 0; j < proc->page_table->size; j++) {
                    if (proc->page_table->table[j].v_index == 0) {
                        proc->page_table->table[j].v_index = first_lv;
                        proc->page_table->table[j].next_lv = malloc(sizeof(struct trans_table_t));
                        proc->page_table->table[j].next_lv->size = 0;
                        trans_table = proc->page_table->table[j].next_lv;
                        proc->page_table->size++;
                        break;
                    }
                }
            }

            // Thêm mục vào bảng phân trang cấp 2
            trans_table->table[trans_table->size].v_index = second_lv;
            trans_table->table[trans_table->size].p_index = prev_page;
            trans_table->size++;
        }

	}
	pthread_mutex_unlock(&mem_lock);




	return ret_mem;
}

int free_mem(addr_t address, struct pcb_t * proc) {
	/* DO NOTHING HERE. This mem is obsoleted */

	pthread_mutex_lock(&mem_lock);

	// tiem bang phan trang cap 1
	addr_t first_lv = get_first_lv(address);
	addr_t second_lv = get_second_lv(address);

	// tiem bang phan trang cap 2
	struct trans_table_t *trans_table = get_trans_table(first_lv, proc->page_table);

	if(trans_table == NULL){
		pthread_mutex_unlock(&mem_lock);
		return 1;
	}

	// Tìm trang vật lý đầu tiên
    int page_index = -1;
	int i = 0;
    for (;i < trans_table->size; i++) {
        if (trans_table->table[i].v_index == second_lv) {

            page_index = trans_table->table[i].p_index;

            // Xóa mục khỏi bảng phân trang cấp 2
			int j = i;
            for ( ;j < trans_table->size - 1; j++) {
                trans_table->table[j] = trans_table->table[j + 1];
            }
            trans_table->size--;
            break;
        }
    }

	return 0;
}

int read_mem(addr_t address, struct pcb_t * proc, BYTE * data) {
	addr_t physical_addr;
	if (translate(address, &physical_addr, proc)) {
		*data = _ram[physical_addr];
		return 0;
	}else{
		return 1;
	}
}

int write_mem(addr_t address, struct pcb_t * proc, BYTE data) {
	addr_t physical_addr;
	if (translate(address, &physical_addr, proc)) {
		_ram[physical_addr] = data;
		return 0;
	}else{
		return 1;
	}
}

void dump(void) {
	int i;
	for (i = 0; i < NUM_PAGES; i++) {
		if (_mem_stat[i].proc != 0) {
			printf("%03d: ", i);
			printf("%05x-%05x - PID: %02d (idx %03d, nxt: %03d)\n",
				i << OFFSET_LEN,
				((i + 1) << OFFSET_LEN) - 1,
				_mem_stat[i].proc,
				_mem_stat[i].index,
				_mem_stat[i].next
			);
			int j;
			for (	j = i << OFFSET_LEN;
				j < ((i+1) << OFFSET_LEN) - 1;
				j++) {
				
				if (_ram[j] != 0) {
					printf("\t%05x: %02x\n", j, _ram[j]);
				}
					
			}
		}
	}
}


