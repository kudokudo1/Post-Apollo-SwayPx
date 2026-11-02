#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "sway/stage_director_process_evidence.h"

static void synthetic(char *buffer, size_t size,
		int pid, const char *comm, int ppid, const char *start) {
	int n = snprintf(buffer, size, "%d (%s) R %d", pid, comm, ppid);
	assert(n > 0 && (size_t)n < size);
	for (int field=5; field<22; ++field) {
		n = snprintf(buffer + strlen(buffer), size - strlen(buffer), " 0");
		assert(n > 0);
	}
	n = snprintf(buffer + strlen(buffer), size - strlen(buffer),
		" %s\n", start);
	assert(n > 0);
}

static void test_proc_parser(void) {
	char s[512] = {0};
	int64_t parent = 0;
	uint64_t start = 0;
	synthetic(s,sizeof(s),321,"browser (e) foo) bar",123,"123456");
	assert(stage_director_parse_linux_proc_stat(s,321,&parent,&start));
	assert(parent==123);
	assert(start==123456);
	assert(!stage_director_parse_linux_proc_stat(s,999,&parent,&start));
	assert(!stage_director_parse_linux_proc_stat(s,321,NULL,&start));
	assert(!stage_director_parse_linux_proc_stat(s,321,&parent,NULL));
	assert(!stage_director_parse_linux_proc_stat("321 browser R 123",321,
		&parent,&start));
	synthetic(s,sizeof(s),321,"test",0,"10");
	assert(!stage_director_parse_linux_proc_stat(s,321,&parent,&start));
	synthetic(s,sizeof(s),321,"test",123,"0");
	assert(!stage_director_parse_linux_proc_stat(s,321,&parent,&start));
	synthetic(s,sizeof(s),321,"test",123,"-10");
	assert(!stage_director_parse_linux_proc_stat(s,321,&parent,&start));
	synthetic(s,sizeof(s),321,"test",123,"18446744073709551616");
	assert(!stage_director_parse_linux_proc_stat(s,321,&parent,&start));
}

static void test_real_readonly_kernel_identity(void) {
	struct stage_director_process_identity self = {0};
	assert(stage_director_read_linux_process(getpid(),&self,NULL));
	assert(self.pid==getpid());
	assert(self.parent_pid==getppid());
	assert(self.start_time_ticks>0);
	assert(self.uid==getuid());
	assert(!stage_director_read_linux_process(0,&self,NULL));
	assert(!stage_director_read_linux_process(-1,&self,NULL));
	assert(!stage_director_read_linux_process(getpid(),NULL,NULL));
	assert(!stage_director_read_linux_process(INT64_MAX,&self,NULL));
}

int main(void) {
	test_proc_parser();
	test_real_readonly_kernel_identity();
	puts("Stage Director NEWS Linux /proc starttime parser and native observation: PASS");
	return 0;
}
