#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>

#ifndef WIN32

#include <fcntl.h>     /* For O_* constants */
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#endif

#include "shmq.h"

using namespace std;

bool shm_circular_queue::create(const string& name, uint32_t cq_size, uint32_t data_max_sz)
{
	assert((cq_size < k_shm_cq_max_sz)
			&& (cq_size >= (data_max_sz + sizeof(shm_block))));

	if (name.size() >= k_shmq_name_sz) {
		errno = ENAMETOOLONG;
		return false;
	}

	uint32_t size = cq_size + sizeof(shm_cq);

#ifndef WIN32
	int shmfd = shm_open(name.c_str(), O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (shmfd == -1) {
		return false;
	}

	int ret = ftruncate(shmfd, size);
	if (ret == -1) {
		close(shmfd);
		shm_unlink(name.c_str());
		return false;
	}

	shm_cq* cq = reinterpret_cast<shm_cq*>(mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0));
	close(shmfd);
	if (cq == MAP_FAILED) {
		shm_unlink(name.c_str());
		return false;
	}
#else
	mapfile_ = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name.c_str());
	if (!mapfile_) {
		return false;
	}

	shm_cq* cq = reinterpret_cast<shm_cq*>(MapViewOfFile(mapfile_, FILE_MAP_ALL_ACCESS, 0, 0, 0));
	if (!cq) {
		CloseHandle(mapfile_);
		mapfile_ = 0;
		return false;
	}
#endif

	cq_					= cq;
	// init shared-memory circular queue
	cq_->head			= sizeof(shm_cq);
	cq_->tail			= sizeof(shm_cq);
	cq_->shm_size		= size;
	cq_->elem_max_sz	= data_max_sz + sizeof(shm_block);
	strcpy(cq_->name, name.c_str());

	return true;
}

bool shm_circular_queue::destroy()
{
#ifndef WIN32
	return ((shm_unlink(cq_->name) == 0) && detach());
#else
	// no easy way to remove shared memory under windows system
	return (CloseHandle(mapfile_) && detach());
#endif
}

bool shm_circular_queue::attach(const string& name)
{
#ifndef WIN32
	int shmfd = shm_open(name.c_str(), O_RDWR, 0);
	if (shmfd == -1) {
		return false;
	}

	// this call of mmap is used to get cq->shm_size only
	shm_cq* cq = reinterpret_cast<shm_cq*>(mmap(0, sizeof(shm_cq), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0));
	if (cq == MAP_FAILED) {
		close(shmfd);
		return false;
	}

	// used to mmap shmfd again
	uint32_t sz = cq->shm_size;
	// unmap cq
	munmap(cq, sizeof(shm_cq));
	// mmap again with the real length of shm_cq
	cq = reinterpret_cast<shm_cq*>(mmap(0, sz, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0));
	close(shmfd);
	if (cq == MAP_FAILED) {
		return false;
	}

	cq_ = cq;
	return true;
#else
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
	if (!hMapFile) {
		return false;
	}

	shm_cq* cq = reinterpret_cast<shm_cq*>(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0));
	CloseHandle(hMapFile);
	if (cq) {
		cq_ = cq;
		return true;
	}

	return false;
#endif
}

bool shm_circular_queue::detach()
{
	if (cq_) {
#ifndef WIN32
		return (munmap(cq_, cq_->shm_size) == 0);
#else
		return UnmapViewOfFile(cq_);
#endif
	}
	return true;
}

uint32_t shm_circular_queue::pop(void** data)
{
	// queue is empty
	if (empty()) {
		return 0;
	}
	align_head();
	// queue is empty
	if (empty()) {
		return 0;
	}

	shm_block* cur_mb = head();
	assert(cur_mb->len <= cq_->elem_max_sz);
	*data = cur_mb->data;
	cq_->head += cur_mb->len;
	return cur_mb->len - sizeof(shm_block);
}

bool shm_circular_queue::push(const void* data, uint32_t len)
{
	assert((len > 0) && (len <= cq_->elem_max_sz - sizeof(shm_block)));

	uint32_t elem_len = len + sizeof(shm_block);
	if (align_tail(elem_len)) {
		shm_block* next_mb = tail();
		next_mb->len = elem_len;
		memcpy(next_mb->data, data, len);
		cq_->tail += elem_len;
		return true;
	}

	return false;
}

bool shm_circular_queue::align_tail(uint32_t len)
{
	uint32_t tail_pos = cq_->tail;
	uint32_t surplus = cq_->shm_size - tail_pos;
	if (surplus >= len) {
		return push_wait(len);
	}

	if (tail_align_wait()) {
		if (surplus >= sizeof(shm_block)) {
			shm_block* pad = tail();
			pad->len = 0xFFFFFFFF;
		}
		cq_->tail = sizeof(shm_cq);
		return push_wait(len);
	}

	return false;
}
