#include <linux/fs.h>
#include <linux/f2fs_fs.h>
#include <linux/namei.h>
#include "f2fs.h"

static char *find_target_name_ci(struct qstr *d_name,
	struct f2fs_dentry_ptr *d, struct fscrypt_str *fstr)
{
	struct f2fs_dir_entry *de;
	struct fscrypt_str de_name = FSTR_INIT(NULL, 0);
	unsigned int bit_pos = 0;

	while (bit_pos < d->max) { /*lint !e574*/
		bit_pos = find_next_bit_le(d->bitmap, d->max, bit_pos);
		if (bit_pos >= d->max) /*lint !e574*/
			break;

		de = &d->dentry[bit_pos];

		if (unlikely(!de->name_len)) {
			bit_pos++;
			continue;
		}

		de_name.len = le16_to_cpu(de->name_len);
		de_name.name = d->filename[bit_pos];

		if (fstr != NULL) {
			int saved_len = fstr->len;
			int err = fscrypt_fname_disk_to_usr(d->inode,
				(u32)de->hash_code, 0, &de_name, fstr);

			if (err)
				return ERR_PTR(err);

			de_name = *fstr;
			fstr->len = saved_len;
		}

		if (de_name.len == d_name->len &&
			!strncasecmp(de_name.name, d_name->name, de_name.len))
			return de_name.name;
		bit_pos += GET_DENTRY_SLOTS(le16_to_cpu(de->name_len));
	}
	return NULL;
}

static int handle_dir_ci_res_name(char *res, char *case_exact, int len)
{
	if (res == NULL)
		return -ENOENT;
	else if (IS_ERR(res))
		return PTR_ERR(res);

	if (case_exact != NULL) {
		memcpy(case_exact, res, len);
		case_exact[len] = '\0';
	}
	return 0;
}

static int find_in_inline_dir_ci(
	struct inode *dir,
	struct qstr *name,
	struct fscrypt_str *fstr,
	char *case_exact_name
) {
	int err;
	struct f2fs_sb_info *sbi = F2FS_SB(dir->i_sb);
	struct f2fs_inline_dentry *inline_dentry;
	struct f2fs_dentry_ptr d;
	struct page *ipage;

	ipage = get_node_page(sbi, dir->i_ino);
	if (IS_ERR(ipage))
		return PTR_ERR(ipage);

	inline_dentry = inline_data_addr(ipage);
	make_dentry_ptr(dir, &d, (void *)inline_dentry, 2);

	err = handle_dir_ci_res_name(
		find_target_name_ci(name, &d, fstr),
		case_exact_name, name->len);

	f2fs_put_page(ipage, 1);
	return err;
}

/*
 * since find_entry_ci will read all the dir when doing creation.
 * it is better to prefetch the whole dir
 */
static void dir_readahead_cache(struct inode *dir)
{
	struct file_ra_state ra;

	file_ra_state_init(&ra, dir->i_mapping);
	page_cache_sync_readahead(dir->i_mapping, &ra,
		NULL, 0, dir_blocks(dir));
}

/* inode_lock(dir) must be held by the caller */
int f2fs_find_entry_ci(struct inode *dir,
	struct qstr *name,
	char *case_exact_name
) {
	struct fscrypt_str *fstr, _fstr = FSTR_INIT(NULL, 0);
	int err;
	bool fname_encrypted = false;

	if (name->len > F2FS_NAME_LEN)
		return -ENAMETOOLONG;

	if (!f2fs_encrypted_inode(dir))
		fstr = NULL;
	else {
		err = fscrypt_get_encryption_info(dir);
		if (err) {
			if (err != -ENOKEY)
				return err;
			else {
				/* the encrypted key is not available */
				fname_encrypted = true;

				if (F2FS_I(dir)->last_ci_name != NULL)
					F2FS_I(dir)->last_ci_name[0] = '\0';
			}
		}
		fstr = &_fstr;
	}

	if (F2FS_I(dir)->last_ci_name != NULL
		&& !strcasecmp(name->name, F2FS_I(dir)->last_ci_name))
		return -ENOENT;

	if (fstr) {
		err = fscrypt_fname_alloc_buffer(dir, F2FS_NAME_LEN, fstr);
		if (err < 0)
			return err;
	}

	if (f2fs_has_inline_dentry(dir))
		err = find_in_inline_dir_ci(dir, name, fstr, case_exact_name);
	else {
		unsigned long npages = dir_blocks(dir);

		if (!npages)
			err = -ENOENT;
		else {
			unsigned long n = 0;

			/* readahead the whole dir */
			dir_readahead_cache(dir);

			do {
				struct page *dentry_page;
				struct f2fs_dentry_block *dentry_blk;
				struct f2fs_dentry_ptr d;

				/* no need to allocate new dentry pages to all the indices */
				dentry_page = find_data_page(dir, n);
				if (dentry_page == ERR_PTR(-ENOENT)) {
					err = -ENOENT;
					continue;
				} else if (IS_ERR(dentry_page)) {
					err = PTR_ERR(dentry_page);
					break;
				}

				dentry_blk = kmap(dentry_page);
				make_dentry_ptr(dir, &d, (void *)dentry_blk, 1);

				err = handle_dir_ci_res_name(
					find_target_name_ci(name, &d, fstr),
					case_exact_name, name->len);

				kunmap(dentry_page);
				f2fs_put_page(dentry_page, 0);
			} while(++n < npages && err == -ENOENT);
		}
	}

	if (fstr)
		fscrypt_fname_free_buffer(fstr);

	/*
	 * the fname will only be record only if the ci_fname
	 * isn't found and encrypted.
	 */
	if (err == -ENOENT && !fname_encrypted) {
		if (unlikely(F2FS_I(dir)->last_ci_name == NULL))
			F2FS_I(dir)->last_ci_name = kmalloc(NAME_MAX + 1, GFP_KERNEL);

		if (F2FS_I(dir)->last_ci_name != NULL) {
			/* copy the trailing '\0' as well */
			memcpy(F2FS_I(dir)->last_ci_name,
				name->name, name->len + 1);
		}
	}

	return err;
}

