/*
 *	linux/mm/mlock.c
 *
 *  (C) Copyright 1995 Linus Torvalds
 *  (C) Copyright 2002 Christoph Hellwig
 */

#include <linux/mman.h>
#include <linux/mm.h>


static int mlock_fixup(struct vm_area_struct * vma, 
	unsigned long start, unsigned long end, unsigned int newflags)
{
	struct mm_struct * mm = vma->vm_mm;
	int pages;
	int ret = 0;

	if (newflags == vma->vm_flags)
		goto out;

	if (start != vma->vm_start) {
		if (split_vma(mm, vma, start, 1)) {
			ret = -EAGAIN;
			goto out;
		}
	}

	if (end != vma->vm_end) {
		if (split_vma(mm, vma, end, 0)) {
			ret = -EAGAIN;
			goto out;
		}
	}

	/*
	 * vm_flags is protected by the mmap_sem held in write mode.
	 * It's okay if try_to_unmap_one unmaps a page just after we
	 * set VM_LOCKED, make_pages_present below will bring it back.
	 */
	vma->vm_flags = newflags;

	/*
	 * Keep track of amount of locked VM.
	 */
	pages = (end - start) >> PAGE_SHIFT;
	if (newflags & VM_LOCKED) {
		pages = -pages;
		ret = make_pages_present(start, end);
	}

	vma->vm_mm->locked_vm -= pages;
out:
	return ret;
}

static int do_mlock(unsigned long start, size_t len, int on)
{
	unsigned long nstart, end, tmp;
	struct vm_area_struct * vma, * next;
	int error;

	len = PAGE_ALIGN(len);
	end = start + len;
	if (end < start)
		return -EINVAL;
	if (end == start)
		return 0;
	vma = find_vma(current->mm, start);
	if (!vma || vma->vm_start > start)
		return -ENOMEM;

	for (nstart = start ; ; ) {
		unsigned int newflags;

		/* Here we know that  vma->vm_start <= nstart < vma->vm_end. */

		newflags = vma->vm_flags | VM_LOCKED;
		if (!on)
			newflags &= ~VM_LOCKED;

		if (vma->vm_end >= end) {
			error = mlock_fixup(vma, nstart, end, newflags);
			break;
		}

		tmp = vma->vm_end;
		next = vma->vm_next;
		error = mlock_fixup(vma, nstart, tmp, newflags);
		if (error)
			break;
		nstart = tmp;
		vma = next;
		if (!vma || vma->vm_start != nstart) {
			error = -ENOMEM;
			break;
		}
	}
	return error;
}

asmlinkage long sys_mlock(unsigned long start, size_t len)
{
	unsigned long locked;
	unsigned long lock_limit;
	int error = -ENOMEM;

	if (!can_do_mlock())
		return -EPERM;

	down_write(&current->mm->mmap_sem);
	len = PAGE_ALIGN(len + (start & ~PAGE_MASK));
	start &= PAGE_MASK;

	locked = len >> PAGE_SHIFT;
	locked += current->mm->locked_vm;

	lock_limit = current->rlim[RLIMIT_MEMLOCK].rlim_cur;
	lock_limit >>= PAGE_SHIFT;

	/* check against resource limits */
	if ((locked <= lock_limit) || capable(CAP_IPC_LOCK))
		error = do_mlock(start, len, 1);
	up_write(&current->mm->mmap_sem);
	return error;
}

asmlinkage long sys_munlock(unsigned long start, size_t len)
{
	int ret;

	down_write(&current->mm->mmap_sem);
	len = PAGE_ALIGN(len + (start & ~PAGE_MASK));
	start &= PAGE_MASK;
	ret = do_mlock(start, len, 0);
	up_write(&current->mm->mmap_sem);
	return ret;
}

static int do_mlockall(int flags)
{
	struct vm_area_struct * vma;
	unsigned int def_flags = 0;

	if (flags & MCL_FUTURE)
		def_flags = VM_LOCKED;
	current->mm->def_flags = def_flags;
	if (flags == MCL_FUTURE)
		goto out;

	for (vma = current->mm->mmap; vma ; vma = vma->vm_next) {
		unsigned int newflags;

		newflags = vma->vm_flags | VM_LOCKED;
		if (!(flags & MCL_CURRENT))
			newflags &= ~VM_LOCKED;

		/* Ignore errors */
		mlock_fixup(vma, vma->vm_start, vma->vm_end, newflags);
	}
out:
	return 0;
}

asmlinkage long sys_mlockall(int flags)
{
	unsigned long lock_limit;
	int ret = -EINVAL;

	if (!flags || (flags & ~(MCL_CURRENT | MCL_FUTURE)))
		goto out;

	ret = -EPERM;
	if (!can_do_mlock())
		goto out;

	down_write(&current->mm->mmap_sem);

	lock_limit = current->rlim[RLIMIT_MEMLOCK].rlim_cur;
	lock_limit >>= PAGE_SHIFT;

	ret = -ENOMEM;
	if (!(flags & MCL_CURRENT) || (current->mm->total_vm <= lock_limit) ||
	    capable(CAP_IPC_LOCK))
		ret = do_mlockall(flags);
	up_write(&current->mm->mmap_sem);
out:
	return ret;
}

asmlinkage long sys_munlockall(void)
{
	int ret;

	down_write(&current->mm->mmap_sem);
	ret = do_mlockall(0);
	up_write(&current->mm->mmap_sem);
	return ret;
}

/*
 * Objects with different lifetime than processes (SHM_LOCK and SHM_HUGETLB
 * shm segments) get accounted against the user_struct instead.
 */
static spinlock_t shmlock_user_lock = SPIN_LOCK_UNLOCKED;

int user_shm_lock(size_t size, struct user_struct *user)
{
	unsigned long lock_limit, locked;
	int allowed = 0;

	spin_lock(&shmlock_user_lock);
	locked = size >> PAGE_SHIFT;
	lock_limit = current->rlim[RLIMIT_MEMLOCK].rlim_cur;
	lock_limit >>= PAGE_SHIFT;
	if (locked + user->locked_shm > lock_limit && !capable(CAP_IPC_LOCK))
		goto out;
	get_uid(user);
	user->locked_shm += locked;
	allowed = 1;
out:
	spin_unlock(&shmlock_user_lock);
	return allowed;
}

void user_shm_unlock(size_t size, struct user_struct *user)
{
	spin_lock(&shmlock_user_lock);
	user->locked_shm -= (size >> PAGE_SHIFT);
	spin_unlock(&shmlock_user_lock);
	free_uid(user);
}
