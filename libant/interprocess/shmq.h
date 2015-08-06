// Copyright (C) 2001-2013 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/*
 *
 * Copyright (C) 2015
 * Antigloss Huang (https://github.com/antigloss)
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

/**
 * @file interprocess/shmq.h
 * @brief Shared memory queues.
 * @platform Linux and Windows.
 * @note Tested heavily only under x86_x64 GNU/Linux.
 * @author Antigloss Huang (https://github.com/antigloss)
 */

#ifndef LIBANT_INTERPROCESS_SHMQ_H_
#define LIBANT_INTERPROCESS_SHMQ_H_

#include <stdint.h>
#include <string>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/**
 * @brief A lock-free shared memory circular queue dedicated to work well
 * 			with only one producer and one consumer, no matter they are
 * 			within the same/different thread/process.
 */
class shm_circular_queue {
public:
	/**
	 * @brief Default constructor. Only do object initialization here,
	 * 			shared memory is not allocated.
	 * 			To make the shm_circular_queue object work properly
	 * 			as a circular queue, call create or attach.
	 * @see create, attach
	 */
	shm_circular_queue()
	{
		cq_ = 0;
#ifdef WIN32
		mapfile_ = 0;
#endif
	}

	/**
	 * @brief Detaches from the circular queue. The shared memory allocated is not remove
	 * 			from the system and can be attached again. To remove the shared memory
	 * 			from the system use destroy().
	 * @note Under Windows, the shared memory will be remove after program termination.
	 */
	~shm_circular_queue()
	{
		detach();
	}

	/**
	 * @brief Allocates shared memory for shm_circular_queue.
	 * 			Failed if the named shared memory already exists.
	 * @param name Name of the shared memory, must be less than 64 bytes.
	 * 				Failed if the named shared memory already exists.
	 * @param cq_size Size of the circular queue, must be 100 bytes greater
	 * 					than `data_max_sz` and less than 2,000,000,000 bytes.
	 * @param data_max_sz Max size in bytes allowed for data pushed into the circular queue.
	 * @return true on success, false on failure.
	 * @see destroy, attach
	 */
	bool create(const std::string& name, uint32_t cq_size, uint32_t data_max_sz);
	/**
	 * @brief Destroy circular queue and remove the shared memory from the system.
	 * @return true on success, false on failure.
	 * @see create
	 */
	bool destroy();
	/**
	 * @brief Attaches to an already created shared memory circular queue.
	 * @param name Name of the shared memory.
	 * @return true on success, false on failure.
	 * @note You must make sure that create() had returned successfully
	 *			before calling this function.
	 * @see create, detach
	 */
	bool attach(const std::string& name);
	/**
	 * @brief Detaches from the circular queue. The shared memory allocated is not remove
	 * 			from the system and can be attached again. To remove the shared memory
	 * 			from the system use destroy().
	 * @return true on success, false on failure.
	 * @note Under Windows, the shared memory will be remove after program termination.
	 * @see attach, destroy
	 */
	bool detach();
	/**
	 * @brief Pops a data element from the circular queue.
	 * 			If success, `*data` will point to the popped data.
	 * 			You DON'T need to free/delete `*data`.
	 * @param data If success, `*data` will point to the popped data.
	 * @return Length of the popped data on success, 0 if the queue is empty.
	 * @see push
	 */
	uint32_t pop(void** data);
	/**
	 * @brief Pushed a data element into the circular queue.
	 * @param data data to be pushed into the circular queue.
	 * @param len Length of the data to be pushed.
	 * @return true on success, false on failure (the queue is full).
	 * @see pop
	 */
	bool push(const void* data, uint32_t len);
	/**
	 * @brief Returns true if the %shm_circular_queue is empty.
	 * @return true if empty, false otherwise.
	 */
	bool empty() const
	{
		return (cq_->head == cq_->tail);
	}

private:
	/*! 64: Max size in bytes for name of the shared memory, including '\0' */
	static const uint32_t k_shmq_name_sz	= 64;
	/* shared memory circular queue max size */
	static const uint32_t k_shm_cq_max_sz	= 2000000000;

private:
	/**
	 * @struct shm_cq
	 * @brief Head of the circular queue
	 */
	struct shm_cq {
		/*! Offset for head of the circular queue */
		volatile uint32_t	head;
		/*! Offset for tail of the circular queue */
		volatile uint32_t	tail;
		/*! Size of the allocated shared memory in bytes */
		uint32_t			shm_size;
		/*! Max size in bytes allowed for data pushed into the circular queue */
		uint32_t			elem_max_sz;
		/*! Name of the shared memory */
		char				name[k_shmq_name_sz];
	};

#pragma pack(1)

	struct shm_block {
		uint32_t	len;
		uint8_t		data[];
	};

#pragma pack()

private:
	// forbid copy and assignment
	shm_circular_queue(const shm_circular_queue&);
	shm_circular_queue& operator=(const shm_circular_queue&);

	shm_block* head() const
	{
		return reinterpret_cast<shm_block*>(reinterpret_cast<char*>(cq_) + cq_->head);
	}

	shm_block* tail() const
	{
		return reinterpret_cast<shm_block*>(reinterpret_cast<char*>(cq_) + cq_->tail);
	}

	bool push_wait(uint32_t len) const
	{
		uint32_t tail = cq_->tail;
		for (int cnt = 0; cnt != 10; ++cnt) {
			uint32_t head = cq_->head;
			// q->elem_max_sz is added just to prevent overwriting
			// the buffer that might be referred to currently
			if ((head <= tail) || (head >= (tail + len + cq_->elem_max_sz))) {
				return true;
			}
#ifndef WIN32
			usleep(5);
#else
			Sleep(1);
#endif
		}

		return false;
	}

	bool tail_align_wait() const
	{
		uint32_t tail = cq_->tail;
		for (int cnt = 0; cnt != 10; ++cnt) {
			uint32_t head = cq_->head;
			if ((head > sizeof(shm_cq)) && (head <= tail)) {
				return true;
			}
#ifndef WIN32
			usleep(5);
#else
			Sleep(1);
#endif
		}

		return false;
	}

	void align_head()
	{
		uint32_t head = cq_->head;
		if (head < cq_->tail) {
			return;
		}

		shm_block* pad = this->head();
		if (((cq_->shm_size - head) < sizeof(shm_block)) || (pad->len == 0xFFFFFFFF)) {
			cq_->head = sizeof(shm_cq);
		}
	}

	bool align_tail(uint32_t len);

private:
	shm_cq*	cq_;
#ifdef WIN32
	HANDLE	mapfile_;
#endif
};

#endif // LIBANT_INTERPROCESS_SHMQ_H_
